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

/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"

#define _XOPEN_SOURCE 700
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "mlle_cr_decrypt.h"

#define BUF_SIZE 10*1024*1024
char in_buf[BUF_SIZE];
char out_buf[BUF_SIZE];
extern FILE* mlle_log;
int main(int argc, char** argv)
{
    FILE* in = NULL;
    FILE* out = NULL;
    int res = 1;
    int bytes = 0;
    mlle_cr_context *c;
    char path[4096];

    mlle_log = stderr;

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <encrypted basedir> <encrypted file> <cleartext file>\n", argv[0]);
        return 1;
    }

    c = mlle_cr_create(argv[1]);
    if (NULL == c) {
        fprintf(stderr, "Could not create decryption context object\n");
    }

    /* OpenSSL initialization stuff. */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    /* Open input and output files. */
    snprintf(path, sizeof(path), "%s/%s", argv[1], argv[2]);
    in = fopen(path, "rb");
    if (in == NULL)
        fprintf(stderr, "Could not open file %s for reading. Error message was: %s\n", path, strerror(errno));
    out = fopen(argv[3], "wb");
    if (out == NULL)
        fprintf(stderr, "Could not open file %s for writing. Error message was: %s\n", argv[3], strerror(errno));

    /* Decrypt file. */
    if (in && out) {
        bytes = (int) fread(in_buf, 1, BUF_SIZE, in);

        bytes = mlle_cr_decrypt(c, argv[2], in_buf, bytes, out_buf);
        if (bytes >= 0) {
            if (fwrite(out_buf , 1, bytes, out) == bytes)
                res = 0;
        }
        if (res > 0) {
            fprintf(stderr, "Decryption failed for file %s.\n", argv[1]);
        }
    }

    /* Close files. */
    if (in != NULL)
        fclose(in);
    if (out != NULL)
        fclose(out);
    if (out != NULL && res != 0)
        remove(argv[2]);

    /* OpenSSL cleanup stuff. */
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    mlle_cr_free(c);

    return res;
}
