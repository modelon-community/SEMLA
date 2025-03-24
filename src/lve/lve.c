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
#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* libcrypto-compat.h must be first */
#include "asprintf.h"
#include "libcrypto-compat.h"
#include "mlle_io.h"
#include "mlle_lve.h"
#include "mlle_lve_feature.h"
#include "mlle_lve_libpath.h"
#include "mlle_parse_command.h"
#include "mlle_protocol.h"

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

/**
 * Returns EXIT_SUCCESS on success. EXIT_FAILURE on failure.
 */
static int _checkout_feature(struct mlle_lve_ctx *lve_ctx,
                             char *checkout_feature, char *libpath)
{
    int result = EXIT_FAILURE;
    size_t libpath_sz = 0;
    char *lib_command_str = NULL;
    char *lib_command_str_dup = NULL;
    size_t lib_command_str_sz = 0;
    size_t checkout_feature_sz = 0;
    char *feature_command_str = NULL;
    char *feature_command_str_dup = NULL;
    size_t feature_command_str_sz = 0;
    struct mlle_command lib_command = {0};
    struct mlle_command *lib_command_ptr = NULL;
    struct mlle_command feature_command = {0};
    struct mlle_command *feature_command_ptr = NULL;
    enum mlle_grammar_error_t grammar_error = LE_UNKNOWN_ERROR;
    char error_msg[ERROR_SIZE] = {'\0'};

    libpath_sz = strlen(libpath);
    lib_command_str_sz = asprintf(
        &lib_command_str, "LIB " MLLE_SIZE_T_FMT "\n%s", libpath_sz, libpath);
    if (-1 == lib_command_str_sz) {
        fprintf(stderr, "Error: Could not allocate memory for lib_command_str");
        goto error;
    }
    lib_command_str_dup =
        strdup(lib_command_str); // mlle_parse_command() modifies its input
    if (lib_command_str_dup == NULL) {
        fprintf(stderr,
                "Error: Could not allocate memory for lib_command_str_dup");
        goto error;
    }
    grammar_error = mlle_parse_command(lib_command_str_dup, &lib_command,
                                       error_msg, ERROR_SIZE);
    if (grammar_error != LE_VALID_GRAMMAR) {
        fprintf(
            stderr,
            "Error: LIB command could not be parsed: error mesage: \"%s\"\n",
            error_msg);
        goto error;
    }
    lib_command_ptr = &lib_command;
    extract_data(&lib_command_ptr, lib_command_str, lib_command_str_sz);
    if (lib_command.data == NULL) {
        fprintf(
            stderr,
            "Error: extract_data() returned NULL for lib_command_str \"%s\"\n",
            lib_command_str);
        goto error;
    }

    checkout_feature_sz = strlen(checkout_feature);
    feature_command_str_sz =
        asprintf(&feature_command_str, "FEATURE " MLLE_SIZE_T_FMT "\n%s",
                 checkout_feature_sz, checkout_feature);
    if (-1 == feature_command_str_sz) {
        fprintf(stderr,
                "Error: Could not allocate memory for feature_command_str");
        goto error;
    }
    feature_command_str_dup =
        strdup(feature_command_str); // mlle_parse_command() modifies its input
    if (feature_command_str_dup == NULL) {
        fprintf(stderr,
                "Error: Could not allocate memory for feature_command_str_dup");
        goto error;
    }
    grammar_error = mlle_parse_command(feature_command_str_dup,
                                       &feature_command, error_msg, ERROR_SIZE);
    if (grammar_error != LE_VALID_GRAMMAR) {
        fprintf(stderr,
                "Error: FEATURE command could not be parsed: error mesage: "
                "\"%s\"\n",
                error_msg);
        goto error;
    }
    feature_command_ptr = &feature_command;
    extract_data(&feature_command_ptr, feature_command_str,
                 feature_command_str_sz);
    if (feature_command.data == NULL) {
        fprintf(stderr,
                "Error: extract_data() returned NULL for feature_command_str "
                "\"%s\"\n",
                feature_command_str);
        goto error;
    }

    mlle_lve_libpath(lve_ctx, &lib_command);
    mlle_lve_feature(lve_ctx, &feature_command);
    result = EXIT_SUCCESS;
error:
    free(lib_command_str);
    free(feature_command_str);
    return result;
}

#ifdef _MSC_VER

/*
 * If we compile using MSVC we have to use WinMain() as entrypoint instead of
 * main() to prevent annoying command prompt pop ups when starting process from
 * a process with a GUI.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    // from https://stackoverflow.com/a/40107581
    return main(__argc, __argv);
}
#endif

int main(int argc, char **argv)
{
    int result = EXIT_FAILURE;
    struct mlle_lve_ctx lve_ctx = {stdin, stdout, NULL, NULL, 0,
                                   0,     0,      NULL, NULL, NULL};

    char *checkout_feature = NULL;
    size_t checkout_feature_sz = 0;
    char *libpath = NULL;
    size_t libpath_sz = 0;
    int i = 0;

    for (i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--checkout-feature")) {
            if (checkout_feature != NULL) {
                fprintf(
                    stderr,
                    "Error: Option --checkout-feature is set more than once");
                goto error;
            }
            i++;
            checkout_feature_sz =
                (strlen(argv[i]) + 1) * sizeof(*checkout_feature);
            checkout_feature = malloc(checkout_feature_sz);
            if (checkout_feature == NULL) {
                fprintf(
                    stderr,
                    "Error: Could not allocate memory for checkout_feature");
                goto error;
            }
            snprintf(checkout_feature, checkout_feature_sz, "%s", argv[i]);
        } else if (0 == strcmp(argv[i], "--libpath")) {
            if (libpath != NULL) {
                fprintf(stderr,
                        "Error: Option --libpath is set more than once");
                goto error;
            }
            i++;
            libpath_sz = (strlen(argv[i]) + 1) * sizeof(*libpath);
            libpath = malloc(libpath_sz);
            if (libpath == NULL) {
                fprintf(stderr, "Error: Could not allocate memory for libpath");
                goto error;
            }
            snprintf(libpath, libpath_sz, "%s", argv[i]);
        } else if (0 == strcmp(argv[i], "--version")) {
            print_version();
            result = argc == 2;
            goto error;
        } else if (0 == strcmp(argv[i], "--help")) {
            print_usage(stdout);
            result = argc == 2;
            goto error;
        } else {
            fprintf(stderr,
                    "Error: Unexpected options. Use '%s --help' for usage.\n",
                    STR(LVETARGET));
            goto error;
        }
    }
    if (checkout_feature != NULL && libpath == NULL) {
        fprintf(
            stderr,
            "Error: Option --checkout-feature is set, which also requires "
            "setting "
            "--libpath, but --libpath is not set. Please also set --libpath.");
        goto error;
    } else if (checkout_feature != NULL && libpath != NULL) {
        result = _checkout_feature(&lve_ctx, checkout_feature, libpath);
    } else {
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
        result = EXIT_SUCCESS;
    }
error:
    free(checkout_feature);
    free(libpath);
    return result;
}
