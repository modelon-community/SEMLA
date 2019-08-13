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
#define _CRT_SECURE_NO_WARNINGS

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"

#include "mlle_protocol.h"
#include "mlle_error.h"
#include "mlle_io.h"
#include "mlle_ssl.h"

#ifdef _WIN32
#define stat _stat
#define SIZE_T_FMT "%Iu"
#else
#define SIZE_T_FMT "%zu"
#endif


char *
mlle_io_read_file(const char *file_path,
                  size_t *file_size,
                  struct mlle_error **error)
{
    FILE *file = NULL;
    struct stat stat_info = { 0 };
    int status = 0;
    size_t size = 0;
    char *file_buffer = NULL;
    size_t bytes_read = 0;
    
#ifdef _WIN32
    #define MAX_FILEPATH_LENGTH_SUPPORTED 4096
    char long_file_path[MAX_FILEPATH_LENGTH_SUPPORTED+1];
    /* On Windows we make extra checks if this is a long path issue */
    size_t file_name_length = strlen(file_path);

#endif

    /* Open file. */
    file = fopen(file_path, "rb");
    if (file == NULL) {
#ifdef _WIN32
        char* ch;
        if (file_name_length > MAX_FILEPATH_LENGTH_SUPPORTED - 4) {
            mlle_error_set(error, 1, 1, "File name length is too long for %s. Cannot open file.", file_path);
            return NULL;
        }

        if ((file_name_length > 255) && strncmp("\\\\?\\", file_path, 4)
            ) {
            /* try to use \\?\ prefix for the long path */
            strncpy(long_file_path, "\\\\?\\", 5);
            strncpy(long_file_path+4, file_path, sizeof(long_file_path)-4);
        }
        else {
            strncpy(long_file_path, file_path, sizeof(long_file_path));
        }
    
        {
            char* ch = long_file_path;
            while (*ch) {
                if (*ch == '/') *ch = '\\';
                ch++;
            }
        }
        file = fopen(long_file_path, "rb");
        if (file == NULL) {            
            if (file_name_length > 255) {
                mlle_error_set(error, 1, 1, "Cannot handle long file name for file %s.", file_path);
            }
            else {
                mlle_error_set(error, 1, 1, "Couldn't open file %s. The error message was: %s",
                    file_path, strerror(errno));
            }
            return NULL;
        }
        file_path = long_file_path;
#else        
        mlle_error_set(error, 1, 1, "Couldn't open file %s. The error message was: %s",
            file_path, strerror(errno));
        return NULL;
#endif
    }

    status = stat(file_path, &stat_info);
#ifdef _WIN32
    // stat cannot handle \\?\ prefix on Windows
    if ((status == -1) && (fseek(file, 0, SEEK_END) == 0)) {
        long int pos = ftell(file);
        rewind(file);
        if ((pos > 0) && (ftell(file) == 0)) {
            status = 0;
            stat_info.st_size = pos;
        }
    }
#endif

    if (status == -1) {
        mlle_error_set(error, 1, 1,
                "Couldn't find file size for %s. The error message was: %s",
                file_path, strerror(errno));
        return NULL;
    }
    size = stat_info.st_size;
    /* Allocate buffer. */
    file_buffer = calloc(size + 1, 1);
    if (file_buffer == NULL) {
        mlle_error_set(error, 1, 1, "Couldn't allocate memory to read file: %s", file_path);
        return NULL;
    }

    /* Read file. */
    bytes_read = fread(file_buffer, 1, size, file);
    file_buffer[bytes_read] = '\0';
    if (bytes_read < size && ferror(file)) {
        mlle_error_set(error, 1, 1,
                "I/O error while reading file %s. The error message was: %s",
                file_path, strerror(errno));
        free(file_buffer);
        file_buffer = NULL;
    }
    *file_size = bytes_read;
    fclose(file);

    return file_buffer;
}


/***************************************
 * Send message of form "<COMMAND>LN".
 ***************************************/
void mlle_send_simple_form(SSL *ssl,
        enum mlle_protocol_command_id command_id)
{
    int result = 0;
    char output[SIMPLE_FORM_BUFFER_SIZE] = {0};

    // Add command to array.
    result = snprintf(output, SIMPLE_FORM_BUFFER_SIZE, "%s\n", mlle_command_info[command_id].name);

    if (result >= 0)
    {
        ssl_write_message(ssl, output, strlen(output));
    }
}

/*************************************************
 * Send message of form "<COMMAND> <NUMBER>LN".
 ************************************************/
int mlle_send_number_form(SSL *ssl,
                      enum mlle_protocol_command_id command_id,
                      long number)
{
    int result = 0;
    char output[NUMBER_FORM_BUFFER_SIZE] = {0};

    // Add command and number to array.
    result = snprintf(output, NUMBER_FORM_BUFFER_SIZE, "%s %ld\n", mlle_command_info[command_id].name, number);

    if (result >= 0)
    {
        result = ssl_write_message(ssl, output, strlen(output));
    }
    return result;
}

void
mlle_send_length_form(SSL *ssl,
                      enum mlle_protocol_command_id command_id,
                      size_t length,  // Length of data.
                      const char *data)
{
    int print_result = 0;
    size_t message_length = 0;
    char *output = NULL;
    size_t output_length;

    // The buffer contains command, length of data and the data.
    output_length = NUMBER_FORM_BUFFER_SIZE + length + 1;
    output = malloc(output_length);
    if (output == NULL)
    {
        return;
    }

    // Add command and length to array.
    print_result = snprintf(output, 
                            output_length, 
                            "%s "SIZE_T_FMT"\n", 
                            mlle_command_info[command_id].name, 
                            length);

    if (print_result >= 0)
    {
        // Add data to array.
        memcpy(output + print_result, data, length);
        message_length = length + print_result;

        // Send array
        ssl_write_message(ssl, output, message_length);
    }

    memset(output, 0, message_length);
    free(output);
}

void
mlle_send_string(SSL *ssl,
                 enum mlle_protocol_command_id command_id,
                 const char* string)
{
    size_t length = strlen(string);

    mlle_send_length_form(ssl, command_id, length, string);
}

void
mlle_send_number_and_length_form(SSL *ssl,
                                 enum mlle_protocol_command_id command_id,
                                 long number,
                                 size_t length,
                                 const char *data)
{
    int print_result = 0;
    size_t message_length = 0;
    char *output = NULL;
    size_t output_length;

    // The buffer contains command, length of data and the data.
    output_length = NUMBER_AND_LENGTH_FORM_BUFFER_SIZE + length + 1;
    output = malloc(output_length);
    if (output == NULL)
    {
        return;
    }

    // Add command, number and length to array.
    print_result = snprintf(output, 
                            output_length, 
                            "%s %ld "SIZE_T_FMT"\n", 
                            mlle_command_info[command_id].name, 
                            number, 
                            length);

    if (print_result >= 0)
    {
        // Add data to array.
        memcpy(output + print_result, data, length);
        message_length = length + print_result;

        // Send array
        ssl_write_message(ssl, output, message_length);
    }

    memset(output, 0, message_length);
    free(output);
}

void
mlle_send_error(SSL *ssl,
                long error_code,
                const char *error_msg)
{
    size_t length = strlen(error_msg);
    if (error_msg != NULL) {
        mlle_send_number_and_length_form(ssl, MLLE_PROTOCOL_ERROR_CMD,
                error_code, length, error_msg);
    } else {
        mlle_send_number_and_length_form(ssl, MLLE_PROTOCOL_ERROR_CMD,
                error_code, 0, "");
    }
}


