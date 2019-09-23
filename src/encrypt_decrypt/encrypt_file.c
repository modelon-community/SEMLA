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
/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "mlle_cr_encrypt.h"
#include "mlle_cr_decrypt.h"
extern FILE* mlle_log;
int main(int argc, char** argv)
{
    FILE* in = NULL;
    FILE* out = NULL;
    int res = 1;
    char pathdest[4096];
    char* destdir = 0;
    char encrypted[4096];
    mlle_log = stderr;
    mlle_cr_context* c;

    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <cleartext file> <encrypted file> [<basedirdest>]\n"
            "<cleartext file> - name of file to encrypt; absolute path\n"
            "<encrypted file> - name of encrypted file; this is relative path if <basedirdest> is given\n"
            "<basedirdest> - <encrypted file> is relative to this directory if given\n",
            argv[0]);
        return 1;
    }

    /* OpenSSL initialization stuff. */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /* Open input and output files. */
    in = fopen(argv[1], "rb");
    if (in == NULL) {
        fprintf(stderr, "Could not open file %s for reading. Error message was: %s\n", argv[1], strerror(errno));
        exit(1);
    }
    strcpy(encrypted, argv[2]);
    if (argc == 4) {
        destdir = argv[3];
        c = mlle_cr_create(destdir);
        snprintf(pathdest, 4096, "%s/%s", destdir, encrypted);
        /* find out what keymask to use by decrypting
        all package.moc recursively if present.
            */
        if (mlle_cr_decrypt(c, encrypted, 0, 0, 0) < 0) {
            fprintf(stderr, "Error getting key masks from parent dirs\n");
            exit(2);
        }
        encrypted[strlen(encrypted) - 1] = '\0';
    }    
    else {
        char* fname = encrypted + strlen(encrypted);
        fname--;
        if (*fname == 'c') {
            *fname = '\0';
        }
        c = mlle_cr_create("");
        snprintf(pathdest, 4096, "%s", encrypted);
        while (fname != encrypted) {
            char ch = *(fname - 1);
            if (ch == '/' || ch == '\\') break;
            fname--;
        }
        strcpy(encrypted, fname);
    }
    
    out = fopen(pathdest, "wb");
    if (out == NULL) {
        fprintf(stderr, "Could not open file %s for writing. Error message was: %s\n", pathdest, strerror(errno));
        exit(2);
    }

    /* Encrypt file. */
    if (in && out) {            
        if (mlle_cr_encrypt(c, encrypted, in, out)) 
            res = 0;
        else
            fprintf(stderr, "Encryption failed for file %s.\n", argv[2]);
    }

    /* Close files. */
    if (in != NULL)
        fclose(in);
    if (out != NULL)
        fclose(out);
    if (out != NULL && res != 0)
        remove(pathdest);

    /* OpenSSL cleanup stuff. */
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();

    return res;
}
