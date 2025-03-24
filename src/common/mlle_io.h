/*
    Copyright (C) 2022 Modelica Association
    Copyright (C) 2015 Modelon AB

    This program is free software: you can redistribute it and/or modify
    it under the terms of the BSD style license.

     This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    BSD_License.txt file for more details.

    You should have received a copy of the BSD_License.txt file
    along with this program. If not, contact Modelon AB
   <http://www.modelon.com>.
*/

#ifndef MLLE_IO_H_
#define MLLE_IO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define _XOPEN_SOURCE 700
#include <stddef.h>
#include <stdio.h>

#include "mlle_error.h"
#include "mlle_lve.h"
#include "mlle_protocol.h"

#define LE_NO_ERROR (0)
#define LE_EOF (-1)
#define LE_LINE_TOO_LONG (-2)
#define LE_IO_ERROR (-3)

// Size of buffers for messages between LVE and Tool.
#define NUMBER_MAX_LEN 20
#define SIMPLE_FORM_BUFFER_SIZE 20
#define NUMBER_FORM_BUFFER_SIZE (SIMPLE_FORM_BUFFER_SIZE + 1 + NUMBER_MAX_LEN)
#define NUMBER_AND_LENGTH_FORM_BUFFER_SIZE                                     \
    (NUMBER_FORM_BUFFER_SIZE + 1 + NUMBER_MAX_LEN)
#define MESSAGE_ERROR_BUFFER_SIZE 100

#ifdef _WIN32
#define MLLE_SIZE_T_FMT "%Iu"
#else
#define MLLE_SIZE_T_FMT "%zu"
#endif

/* code is expected to write out debug messages if mlle_log is not NULL */
extern FILE *mlle_log;

/* Open mlle_log if specified environment variable is defined and points to a
 * filename */
void mlle_log_open(const char *envvar);

char *mlle_io_read_file(const char *file_path, size_t *file_size,
                        struct mlle_error **error);

void mlle_send_simple_form(SSL *ssl, enum mlle_protocol_command_id command_id);

int mlle_send_number_form(SSL *ssl, enum mlle_protocol_command_id command_id,
                          long number);

void mlle_send_length_form(SSL *ssl, enum mlle_protocol_command_id command_id,
                           size_t length, const char *data);

void mlle_send_string(SSL *ssl, enum mlle_protocol_command_id command,
                      const char *string);

void mlle_send_number_and_length_form(SSL *ssl,
                                      enum mlle_protocol_command_id command_id,
                                      long number, size_t length,
                                      const char *data);

void mlle_send_error(SSL *ssl, long error_code, const char *error_msg);

/*
void mlle_send_notsimple();

void mlle_send_tools();

void mlle_send_yes();

void mlle_send_version(long max_protocol_version);

void mlle_send_feature(const char *feature);

void mlle_send_file(const char *file_path);

void mlle_send_filecont(const char *file_contents);

void mlle_send_lib(const char *path);

void mlle_send_license(const char *package_name);

void mlle_send_no(const char *explanation);

void mlle_send_pubkey(const char *public_key);

void mlle_send_toollist(const char *toollist);
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MLLE_IO_H_ */
