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

#define _XOPEN_SOURCE 700
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"
#include "mlle_io.h"
#include "mlle_lve.h"

#ifdef INCLUDE_OPENSSL_APPLINK
#ifndef __INCLUDE_OPENSSL_APPLINK
#define __INCLUDE_OPENSSL_APPLINK
#include <openssl/applink.c>
#endif /* __INCLUDE_OPENSSL_APPLINK */
#endif /* INCLUDE_OPENSSL_APPLINK */

#define STR2(x) #x
#define STR(x) STR2(x)

static void print_version()
{

    printf("{\n"
           "    \"name\": \"%s\",\n"
           "    \"version\": \"%s\",\n"
#ifdef TOOLS_LIST_JSON_STRING
           "    \"tools\": %s,\n"
#endif
           "}\n",
           STR(PROJECT_NAME), STR(PROJECT_VERSION)
#ifdef TOOLS_LIST_JSON_STRING
                                  ,
           TOOLS_LIST_JSON_STRING
#endif
    );
}

static void print_usage(FILE *stream)
{
    fprintf(
        stream,
        "USAGE: %s [options]\n"
        "Handles Decryption and Licensing of (Encrypted) Modelica Libraries.\n"
        "\nOptions:\n"
        "--checkout-feature <name>   License feature to checkout. Also "
        "requires setting --libpath.\n"
        "--libpath <path>   Path to the encrypted Modelica library "
        "(optional).\n"
        "                   Path can be either relative from current directory "
        "or absolute.\n"
        "--version   Print version information and exit. This must be "
        "the only option given.\n"
        "--help   Print usage and exit. This must be the only "
        "option given.\n",
        STR(LVETARGET));
}

#ifdef _MSC_VER

/*
 * If we compile using MSVC we have to use this main method signature to
 * prevent annoying command prompt pop ups when starting process from a
 * process with a GUI.
 */
int main() { return WinMain(0, 0, 0, 0); }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)

#else

int main(int argc, char **argv)
{

#endif

    struct mlle_lve_ctx lve_ctx = {stdin, stdout, NULL, NULL, 0,
                                   0,     0,      NULL, NULL, NULL};

    char *checkout_feature = NULL;
    char *libpath = NULL;
    int i = 0;

    for (i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--checkout-feature")) {
            i++;
            snprintf(checkout_feature, sizeof(checkout_feature), "%s", argv[i]);
        } else if (0 == strcmp(argv[i], "--libpath")) {
            i++;
            snprintf(libpath, sizeof(libpath), "%s", argv[i]);
        } else if (0 == strcmp(argv[i], "--version")) {
            print_version();
            exit(argc == 2);
        } else if (0 == strcmp(argv[i], "--help")) {
            print_usage(stdout);
            exit(argc == 2);
        } else {
            fprintf(stderr,
                    "Error: Unexpected options. Use '%s --help' for usage.\n",
                    STR(LVETARGET));
            exit(1);
        }
    }
    if (checkout_feature != NULL && libpath == NULL) {
        fprintf(
            stderr,
            "Error: --checkout-feature is set, which also requires setting "
            "--libpath, but --libpath is not set. Please also set --libpath.");
        return EXIT_FAILURE;
    } else if (checkout_feature != NULL && libpath != NULL) {

        // mlle_lve_libpath(lve_ctx, command);
    }

    mlle_log_open("SEMLA_LVE_LOG_FILE");

    mlle_lve_init(&lve_ctx);

    // Set upp SSL.
    if (ssl_setup_lve(&lve_ctx)) {
        // Connect with Tool (client).
        if (lve_perform_handshake(&lve_ctx)) {
            // Start receiving data.
            mlle_lve_receive(&lve_ctx);
        }
    }

    mlle_lve_shutdown(&lve_ctx);

    return EXIT_SUCCESS;
}
