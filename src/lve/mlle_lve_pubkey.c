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
#include <string.h>
/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"

#include "mlle_lve.h"
#include "mlle_lve_config.h"
#include "mlle_protocol.h"
#include "mlle_error.h"
#include "mlle_io.h"
#include "mlle_lve_pubkey.h"
#include "public_key_tool.h"

#ifdef MLLE_GLOBAL_LICENSE_FEATURE
#include "mlle_lve_feature.h"
#include "mlle_license_manager.h"
#endif


/* Need both due to sneaky preprocessor behavior. */
#define STR2(x) #x
#define STR(x) STR2(x)

/**************************************************************
 * Validates the public key from a Tool (client) with public
 * tool keys the LVE has access to.
 * The outcome of the validation is set in a variable in the
 * lve structure. If validation failed the lve will still
 * function but not every command can be used.
 *
 * Parameters:
 *      lve_ctx - I/O information for the LVE.
 ************************************************************/
void mlle_lve_validate_pubkey(struct mlle_lve_ctx *lve_ctx)
{
    /* Check tool - start with approved, then test. */
    lve_ctx->tool_approved = 1;

    /* Check the Tool's public key against list of approved tools. */
    lve_ctx->tool_approved = validate();

    if (!lve_ctx->tool_approved)
    {
        lve_ctx->tool_error_msg = "Not a valid tool";
        lve_ctx->tool_error_type = MLLE_PROTOCOL_TOOL_NOT_ALLOWED_ERROR;
        lve_ctx->tool_approved = 0;
    }

    /* Check global licence, if defined. */
#ifdef MLLE_GLOBAL_LICENSE_FEATURE
    if (lve_ctx->tool_approved) {
        struct mlle_error *error = NULL;
        int success = 0;

        success = mlle_lve_setup_licensing(lve_ctx, &error);
        if (success) {
            success = mlle_license_checkout_feature(lve_ctx->lic_mgr,
                    strlen(STR(MLLE_GLOBAL_LICENSE_FEATURE)), STR(MLLE_GLOBAL_LICENSE_FEATURE), &error);
        }
        if (!success) {
            lve_ctx->tool_error_msg = strdup(mlle_error_get_message(error));
            lve_ctx->tool_error_type = MLLE_PROTOCOL_LICENSE_ERROR;
            lve_ctx->tool_approved = 0;
            mlle_error_free(&error);
        }
    }
#endif
}

/*************************************************************
 * Validate the Tool public key against a list of
 * trusted public keys. If key is not valid the Tool is not
 * approved and can't use all the library commands.
 *
 * Returns:
 *      1 - the public key is valid.
 *      0 - the public key is not valid.
 ************************************************************/
int validate()
{
    int result = 1;
    int i;
    DECLARE_PUBLIC_KEY_TOOL();

    // TODO check with several tool keys.
    INITIALIZE_PUBLIC_KEY_TOOL();
    // global_tool_pub_key is the public key received in
    // the LVE:s callback method during TLS handshake.
    for (i = 0; i < PUBLIC_KEY_TOOL_NUM; i++){
        if (strcmp(global_tool_pub_key, PUBLIC_KEY_TOOL[i]) == 0)
        {
            /* Found matching key. */
            break;            
        }
    }
    if (i == PUBLIC_KEY_TOOL_NUM) {
        /* Validation failed. */
        result = 0;
    }
    CLEAR_PUBLIC_KEY_TOOL();

    return result;
}
