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

#ifndef MLLE_LVE_H_
#define MLLE_LVE_H_


#define _XOPEN_SOURCE 700
#include <stddef.h>
#include <stdio.h>
/* mlle_portability.h must be first */
#include "mlle_portability.h"
#include "mlle_protocol.h"
#include "mlle_protocol_lve_state.h"
#include <openssl/ssl.h>
#include "mlle_ssl.h"
#include "mlle_ssl_lve.h"
#include "mlle_utils.h"
#include "mlle_cr_decrypt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MSG_SIZE (MLLE_PROTOCOL_MAX_COMMAND_LINE_SIZE + 1)
#define ERROR_SIZE (4096)
#define PATH_SIZE (2048)
#define MIN_PROTOCOL_VERSION (1)
#define MAX_PROTOCOL_VERSION (1)

struct mlle_lve_ctx {
    FILE *in_stream;
    FILE *out_stream;
    SSL *ssl;
    char *libpath;
    size_t path_size;
    int tool_approved;
    enum mlle_protocol_error_id tool_error_type;
    char *tool_error_msg;
    struct mlle_license *lic_mgr;
    mlle_cr_context *cr_context;
};


void mlle_lve_init(struct mlle_lve_ctx *lve_ctx);

enum mlle_lve_state
mlle_lve_handle_command(struct mlle_lve_ctx *lve_ctx,
                        enum mlle_lve_state current_state,
                        struct mlle_command *command,
                        char *error_msg,
                        size_t error_length,
                        char *buffer,
                        size_t buffer_len);


int mlle_lve_receive(struct mlle_lve_ctx *lve_ctx);

void mlle_lve_shutdown(struct mlle_lve_ctx *lve_ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MLLE_LVE_H_ */
