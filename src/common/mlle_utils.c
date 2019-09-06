/*
    Copyright (C) 2015 Modelon AB

    This program is free software: you can redistribute it and/or modify
    it under the terms of the BSD style license.

     This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    BSD_License.txt file for more details.

    You should have received a copy of the BSD_License.txt file
    along with this program. If not, contact Modelon AB <http://www.modelon.com>.
*/

#define _XOPEN_SOURCE 700
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"
#include "mlle_io.h"
#include "mlle_parse_command.h"

#define SSL_RECORD_SIZE (16400)
#define CMD_BUFFER_SIZE (MLLE_PROTOCOL_MAX_COMMAND_LINE_SIZE + 1)
#define ERROR_SIZE (4096)


/**************************************************************
 * Extract the data part from a message.
 * The command buffer is allocated with total length of data,
 * i.e. the size of the file to read.
 *
 * Parameters:
 *      command - the structure where we will place the
 *                extracted data.
 *      buffer - the message containing command and data.
 *************************************************************/
size_t extract_data(struct mlle_command **command, char *buffer, size_t buffer_len)
{
    char *data_part = NULL;
    size_t data_len = 0;

    if ((*command)->data != NULL)
    {
        free((*command)->data);
        (*command)->data = NULL;
    }

    // Get pointer to data part
    // Everything after the first new line character is the data part.
    data_part = strstr(buffer, "\n");
    if (data_part == NULL)
    {
        return 0;
    }
    data_part++;
    data_len = buffer_len - (data_part - buffer);

    // Allocate memory for total length of data to read.
    (*command)->data = malloc((*command)->length + 1);
    if ((*command)->data == NULL)
    {
        return 0;
    }

    // Copy data
    memcpy((*command)->data, data_part, data_len);
    (*command)->data[(*command)->length] = '\0';

    return data_len;
}

/****************************************
 * Read a command.
 *
 * Returns:
 *      1 - read was successful.
 *      0 - read failed.
 ***************************************/
int mlle_read_command(SSL *ssl,
                  struct mlle_command *command,
                  struct mlle_error **error)
{
    int bytesRead = 0;
    size_t totalBytesRead = 0;
    char *token_buffer = NULL;
    enum mlle_grammar_error_t grammar_error = LE_UNKNOWN_ERROR;
    char error_msg[ERROR_SIZE] = { '\0' };
    char *messageBuffer = NULL;

    // The SSL buffer can hold at most 16 kb of data.
    messageBuffer = calloc(SSL_RECORD_SIZE, 1);
    if (messageBuffer == NULL) 
    {
        mlle_error_set_literal(error, 1, 1, "Failed to allocate memory for the message buffer");
        return 0;
    }

    // Read data from SSL.
    bytesRead = ssl_read_message(ssl, &messageBuffer);

    // Check for error.
    if (bytesRead < 0 || bytesRead > SSL_RECORD_SIZE) 
    {
        ssl_get_error_string(bytesRead, error_msg, ERROR_SIZE);
        mlle_error_set_literal(error, 1, 1, error_msg);
        return 0;
    }

    // Create a temporary array.
    token_buffer = calloc(bytesRead + 1, 1);
    if (token_buffer == NULL) 
    {
        mlle_error_set_literal(error, 1, 1, "Failed to allocate memory for the token buffer");
        return 0;
    }
    memcpy(token_buffer, messageBuffer, bytesRead);
    token_buffer[bytesRead - 1] = 0;

    // Parse command.
    grammar_error = mlle_parse_command(token_buffer, command, error_msg, ERROR_SIZE);

    free(token_buffer);
    token_buffer = NULL;

    if (grammar_error != LE_VALID_GRAMMAR)
    {
        if (grammar_error == LE_UNKNOWN_ERROR)
        {
            mlle_error_set_literal(error, 1, 1, "Internal error in parsing.");
        }
        else
        {
            mlle_error_set_literal(error, 1, 1, error_msg);
        }
        return 0;
    }
    assert(command->id != MLLE_PROTOCOL_UNDEFINED_CMD);

    // Does messages contain a data part.
    if (mlle_command_info[command->id].msg_form == MLLE_PROTOCOL_LENGTH_MSG_FORM
        || mlle_command_info[command->id].msg_form == MLLE_PROTOCOL_NUMBER_AND_LENGTH_MSG_FORM)
    {
        // Extract data and add to command buffer.
        totalBytesRead = extract_data(&command, messageBuffer, bytesRead);

        if (command->data == NULL)
        {
            mlle_error_set_literal(error, 1, 1, "End Of File.");
        }
    }

    // -----------------------------
    // Are there more data to read?
    // -----------------------------
    while (totalBytesRead < command->length)
    {
        // Read more data.
        bytesRead = ssl_read_message(ssl, &messageBuffer);

        // Add data to command buffer.
        memcpy(command->data + totalBytesRead, messageBuffer, bytesRead);
        totalBytesRead += bytesRead;
    }

    return 1;
}
