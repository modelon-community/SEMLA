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
#include <stdlib.h>
#include <string.h>
/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"
#include "mlle_lve.h"
#include "mlle_io.h"
#include "mlle_lve_file.h"
#include "mlle_cr_decrypt.h"

int
mlle_lve_file(struct mlle_lve_ctx *lve_ctx,
              const struct mlle_command *command)
{
    size_t path_size = 0;
    char *rel_file_path = NULL; /* relative file path in the library*/
    char *file_path = NULL;
    enum mlle_protocol_error_id error_code = MLLE_PROTOCOL_UNDEFINED_ERROR;
    char *error_msg = NULL;
    size_t file_size = 0;
    char *file_buffer = NULL;
    char *file_out_buffer = NULL;
    struct mlle_error *error = NULL;
    char *file_extension;
    int decrypted_size = 0;

    if (!lve_ctx->tool_approved) {
        error_code = lve_ctx->tool_error_type;
        error_msg = lve_ctx->tool_error_msg;
        goto CLEANUP;
    }

    /* Concatenate path. */
    path_size = lve_ctx->path_size + command->length + 2; /* 1 extra for a '/', 1 for null char */
    file_path = malloc(path_size);
    if (file_path == NULL) {
        error_code = MLLE_PROTOCOL_OTHER_ERROR;
        error_msg = "Couldn't allocate memory";
        goto CLEANUP;
    }
    rel_file_path = command->data;
    snprintf(file_path, path_size, "%s/%s", lve_ctx->libpath, rel_file_path);

    file_buffer = mlle_io_read_file(file_path, &file_size, &error);
    if (file_buffer == NULL) {
        error_code = MLLE_PROTOCOL_FILE_IO_ERROR;
        error_msg = mlle_error_get_message(error);
        goto CLEANUP;
    }
    file_extension = strrchr(command->data, '.');
    if (file_extension != NULL)
        file_extension++;

    /* If it's an encrypted Modelica file, decrypt it. */
    if (file_extension != NULL
        && strcasecmp(file_extension, MLLE_ENCRYPTED_MODELICA_FILE_EXTENSION) == 0)
    {

        file_out_buffer = malloc(file_size + 1);
        if (file_out_buffer == NULL) {
            error_code = MLLE_PROTOCOL_OTHER_ERROR;
            error_msg = malloc(100 + path_size);
            if (error_msg != NULL) {
                snprintf(error_msg, 100 + path_size, "Could not allocate memory to decrypt file %s.", file_path);
            }
            goto CLEANUP;
        }

        decrypted_size = mlle_cr_decrypt(lve_ctx->cr_context, rel_file_path, file_buffer, file_size, file_out_buffer);
        
        if (decrypted_size < 0) {
            error_code = MLLE_PROTOCOL_OTHER_ERROR;
            error_msg = malloc(100 + path_size);
            if (error_msg != NULL) {
                snprintf(error_msg, 100 + path_size, "Failed to decrypt file %s, might be corrupted.", file_path);
            }
            goto CLEANUP;
        }
        file_size = decrypted_size;
        /* Send file data. */
        mlle_send_length_form(lve_ctx->ssl, MLLE_PROTOCOL_FILECONT_CMD,
            file_size, file_out_buffer);
    }
    else {
        /* Send file data. */
        mlle_send_length_form(lve_ctx->ssl, MLLE_PROTOCOL_FILECONT_CMD,
            file_size, file_buffer);
    }


CLEANUP:
    free(file_buffer);
    free(file_out_buffer);
    free(file_path);
    if (error_msg != NULL) {
        mlle_send_error(lve_ctx->ssl, error_code, error_msg);
    }
    if (error != NULL) {
        mlle_error_free(&error);
    }
    if (decrypted_size < 0) {
        free(error_msg);
    }

    return error_code != MLLE_PROTOCOL_UNDEFINED_ERROR;
}
