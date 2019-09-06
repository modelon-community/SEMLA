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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"

#include "mlle_lve.h"
#include "mlle_protocol.h"
#include "mlle_error.h"
#include "mlle_io.h"
#include "mlle_lve_libpath.h"


/**********************************************************
 * Handles the command for lib.
 * The command is of the form "<LIB> <LENGTH>LN<DATA>.
 *
 * Parameters:
 *      lve_ctx - communication structure.
 *      command - the contents of the message.
 * Returns:
 *      1 - library path was ok.
 *      0 - some error occurred during validation of path.
 *********************************************************/
int mlle_lve_libpath(struct mlle_lve_ctx *lve_ctx,
                 const struct mlle_command *command)
{
    size_t path_size = 0;
    char last = '\0';

    // Is there something to check?
    if ( (command->data == NULL) || (command->length == 0) )
    {
        lve_ctx->tool_error_type = MLLE_PROTOCOL_FILE_NOT_FOUND_ERROR;
        lve_ctx->tool_error_msg = "Path to library is missing.";
        return 0;
    }

    path_size = command->length;

    lve_ctx->libpath = malloc(path_size + 1);
    memcpy(lve_ctx->libpath, command->data, path_size);
    lve_ctx->libpath[path_size] = '\0';

    // Trim trailing slash
    last = lve_ctx->libpath[path_size - 1];
    while (path_size > 1 && (last == '\\' || last == '/')) {
        path_size--;
        lve_ctx->libpath[path_size] = '\0';
        last = lve_ctx->libpath[path_size - 1];
    }

    lve_ctx->path_size = path_size;

    lve_ctx->cr_context = mlle_cr_create(lve_ctx->libpath);
    if (NULL == lve_ctx->cr_context){
        lve_ctx->tool_error_type = MLLE_PROTOCOL_OTHER_ERROR;
        lve_ctx->tool_error_msg = "Failed to allocate memory for decryption context.";
        return 0;
    }

    // Always send a reply.
    mlle_send_simple_form(lve_ctx->ssl, MLLE_PROTOCOL_YES_CMD);

    // Validate the directory path.
    if (!validate_directory(&lve_ctx))
    {
        return 0;
    }

    return 1;
}


/*************************************************************
 * Validate the path and check if it is a directory.
 *
 * Parameters:
 *      lve_ctx - structure containing the path to validate.
 *
 * Returns:
 *      1 - validation succeeded.
 *      0 - validation failed.
 ************************************************************/
int validate_directory(struct mlle_lve_ctx **lve_ctx)
{
    struct stat info;
    int error = 0;

    if (stat((*lve_ctx)->libpath, &info) != 0)
    {
        // Directory does not exist.
        (*lve_ctx)->tool_error_type = MLLE_PROTOCOL_FILE_NOT_FOUND_ERROR;
        (*lve_ctx)->tool_error_msg = "Library path does not exist.";
         return 0;
    }
    else if ((info.st_mode & S_IFDIR) == 0)
    {
        // It's not a directory.
        (*lve_ctx)->tool_error_type = MLLE_PROTOCOL_FILE_NOT_FOUND_ERROR;
        (*lve_ctx)->tool_error_msg = "Library path is not a directory.";
        return 0;
    }

    return 1;
}
