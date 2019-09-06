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
#include <stdio.h>
#include <string.h>
/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include "mlle_cr_crypt.h"
#include "mlle_cr_context.h"
#include "mlle_error.h"

#include "random_key_file.h"

#ifdef _WIN32

/* Arbitrary, but should be "enough". */
#define WIN_PRNG_SEED_SIZE (256)

/*
 * Make sure PRNG is seeded properly.
 * Returns 1 on success, and 0 on failure.
 */
static int mlle_cr_seed() {
    HCRYPTPROV win_crypt_prov;
    int win_seed_res;
    char win_seed_buf[WIN_PRNG_SEED_SIZE];

    if (!RAND_status()) {
        CryptAcquireContext(&win_crypt_prov, NULL, NULL, PROV_RSA_FULL, CRYPT_SILENT);
        win_seed_res = CryptGenRandom(win_crypt_prov, WIN_PRNG_SEED_SIZE, win_seed_buf);
        CryptReleaseContext(win_crypt_prov, 0);
        if (!win_seed_res)
            return 0;
        RAND_seed(win_seed_buf, WIN_PRNG_SEED_SIZE);
    }
    return 1;
}
#else
#define mlle_cr_seed() (1)
#endif

/* Length of buffers to use. */
#define READ_BUF_LEN (256)
#define CRYPT_BUF_LEN (READ_BUF_LEN + 32)


void mlle_debug_log_key(const unsigned char* key);
extern FILE* mlle_log;
/*
    Mask the provided key based on the context and rel_file_path. 
    Returns:
        0 - if only key_mask was updated
        1 - if key_mask and store_mask were updated and store_mask needs to be stored in file (this is for package.mo)
        -1 - on error.
*/
int mlle_mask_key(mlle_cr_context* context, const char* rel_file_path, unsigned char* key, unsigned char* store_mask) {
    char path[MLLE_LONG_FILE_NAME_MAX];
    size_t rel_path_len;

    int last_slash_index = 0;
    int i = 0;
    char ch;
    struct mlle_key_mask_map* key_mask_map = context->keymask_map;
    struct mlle_key_mask_map* map_item = NULL;
    char* parent_key = 0;
    int ret = 0;

    if (mlle_log) {
        fprintf(mlle_log, "mlle_mask_key: got key for %s \n", rel_file_path);
        mlle_debug_log_key(key);
    }


    while (rel_file_path[i] && (i < MLLE_LONG_FILE_NAME_MAX)) {
        ch = rel_file_path[i];
        path[i] = ch;
        if ((ch == '/') || (ch == '\\')) {
            last_slash_index = i;
        }
        i++;
    }
    rel_path_len = i;
    if (!last_slash_index) {
        path[0] = '/';
        path[1] = '\0';
        last_slash_index = 1;
    }
    else {
        path[last_slash_index] = 0;
    }

    if (strcasecmp("package.mo", &(rel_file_path[rel_path_len - PACKAGE_MO_STRLEN])) == 0) {
        HASH_FIND_STR(key_mask_map, path, map_item);
        if (map_item != NULL) {
              /* if this is a package.mo file then there should not be any mask for in the table yet*/
              fprintf(stderr, "Found key mask in the table while working on %s; package.mo must be encrypted first\n", rel_file_path);
              return -1;
        }
        ret = 1;

        if (path[0] != '/') {
            // if in subdirectory - mask based on parent dir
            if (mlle_mask_key(context, path, key, store_mask) != 0) {
                fprintf(stderr, "Unexpected key mask output when processing %s\n", path);
                return -1;
            }
        }

        /* Create and store key mask for this directory */
        RAND_bytes(store_mask, MLLE_CR_KEY_LEN);
        map_item = (struct mlle_key_mask_map*)calloc(1, sizeof(struct mlle_key_mask_map) + rel_path_len + 1);
        if (map_item == NULL) {
            return -1;
        }
        map_item->relpath = map_item->buffer;
        memcpy(map_item->buffer, path, last_slash_index);
        memcpy(map_item->key_mask, store_mask, MLLE_CR_KEY_LEN);

        HASH_ADD_KEYPTR(hh, key_mask_map, map_item->relpath, last_slash_index, map_item);
        context->keymask_map = key_mask_map;

        if (mlle_log) {
            fprintf(mlle_log, "mlle_mask_key: generated and stored mask for %s \n", map_item->relpath);
            mlle_debug_log_key(store_mask);
        }

        return 1;
    }

    HASH_FIND_STR(key_mask_map, path, map_item);
    if (map_item == NULL) {
        /* nothing in the table - grab from parent dir and store */
        map_item = (struct mlle_key_mask_map*)calloc(1, sizeof(struct mlle_key_mask_map) + rel_path_len + 1);
        if (map_item == NULL) {
            fprintf(stderr, "Could not allocate memory when processing %s\n", path);
            return -1;
        }
        map_item->relpath = map_item->buffer;
        memcpy(map_item->buffer, path, last_slash_index);
        
        if (path[0] != '/') {
            /* Recursive call is only done for subdirs; at top level we'll store zeros */
            if (mlle_mask_key(context, path, map_item->key_mask, store_mask) != 0) {
                fprintf(stderr, "Unexpected key mask output when processing %s\n", path);
                free(map_item);
                return -1;
            }
        }
        if (mlle_log) {
            fprintf(mlle_log, "mlle_mask_key: storing mask based on parent for %s \n", map_item->relpath);
            mlle_debug_log_key(map_item->key_mask);
        }

        HASH_ADD_KEYPTR(hh, key_mask_map, map_item->relpath, last_slash_index, map_item);
        context->keymask_map = key_mask_map;
    }

    /* xor the key with the found mask*/
    for (i = 0; i < MLLE_CR_KEY_LEN; ++i) {
        key[i] = key[i] ^ map_item->key_mask[i];
    }
    if (mlle_log) {
        fprintf(mlle_log, "mlle_mask_key: applied key mask for %s \n", rel_file_path);
        mlle_debug_log_key(key);
    }
    return 0;
}


/*
 * Read data from stream in until eof, encrypt store_mask + data, and write IV, encrypted data and HMAC (in that order) to stream out.
 *
 * Returns zero on error.
 */
int mlle_cr_encrypt(mlle_cr_context* context, 
                    const char* rel_file_path,
                    FILE* in, 
                    FILE* out)
{
    /* TODO: Enable better error messages. */
    EVP_CIPHER_CTX *c_ctx;
    HMAC_CTX *h_ctx;
    unsigned char *mac = NULL;
    unsigned char *iv = NULL;
    const EVP_CIPHER *cipher;
    int iv_len;
    const EVP_MD *hash;
    unsigned int mac_len;
    unsigned char read_buf[READ_BUF_LEN];
    unsigned char crypt_buf[CRYPT_BUF_LEN];
    int res = 0;
    int read_len;
    int crypt_len;
    unsigned char store_mask[MLLE_CR_KEY_LEN];
    int store_mask_flag = 0;
    DECLARE_MLLE_CR_KEY();

    /* Init structures. */
    c_ctx = EVP_CIPHER_CTX_new();
    h_ctx = HMAC_CTX_new();
    EVP_CIPHER_CTX_init(c_ctx);

    /* Select algorithms and get parameters. */
    cipher = MLLE_CR_CIPHER;
    iv_len = EVP_CIPHER_iv_length(cipher);
    hash = MLLE_CR_HASH;
    mac_len = (unsigned int) EVP_MD_size(hash);

    if (!mlle_cr_seed())
        goto error;

    /* Create IV and write it to file. */
    iv = (unsigned char*) malloc(iv_len);
    RAND_bytes(iv, iv_len);
    if (fwrite(iv, iv_len, 1, out) < 1)
        goto error;

    /* Set up encryption and HMAC calculation. */
    INITIALIZE_MLLE_CR_KEY();
    store_mask_flag = mlle_mask_key(context, rel_file_path, MLLE_CR_KEY, store_mask);
    if (store_mask_flag < 0) {
        goto error;
    }
    if (!EVP_EncryptInit_ex(c_ctx, cipher, NULL, MLLE_CR_KEY, iv))
        goto error;
    mac = (unsigned char*) malloc(mac_len);
    if (!mac)
        goto error;
    if (!HMAC_Init_ex(h_ctx, MLLE_CR_KEY, MLLE_CR_KEY_LEN, hash, NULL))
        goto error;
    if (!HMAC_Update(h_ctx, iv, iv_len))
        goto error;

    /* Process file. */
    while (!feof(in)) {
        /* Read cleartext data. */
        read_len = (int) fread(read_buf, 1, READ_BUF_LEN, in);
        if (read_len < READ_BUF_LEN && ferror(in))
            goto error;

        /* Encrypt and write data. */
        if (!EVP_EncryptUpdate(c_ctx, crypt_buf, &crypt_len, read_buf, read_len))
            goto error;
        if (fwrite(crypt_buf, 1, crypt_len, out) < (size_t) crypt_len)
            goto error;

        /* Update HMAC calculation. */
        if (!HMAC_Update(h_ctx, read_buf, read_len))
            goto error;
    }

    if (store_mask_flag) {
        /* Encrypt and write store_mask. */
        if (!EVP_EncryptUpdate(c_ctx, crypt_buf, &crypt_len, store_mask, MLLE_CR_KEY_LEN))
            goto error;
        if (fwrite(crypt_buf, 1, crypt_len, out) < (size_t)crypt_len)
            goto error;
        /* Update HMAC calculation. */
        if (!HMAC_Update(h_ctx, store_mask, MLLE_CR_KEY_LEN))
            goto error;
    }

    /* Finalize encryption and write final data. */
    if (!EVP_EncryptFinal_ex(c_ctx, crypt_buf, &crypt_len))
        goto error;
    if (fwrite(crypt_buf, 1, crypt_len, out) < (size_t) crypt_len)
        goto error;

    /* Finalize HMAC calculation and write to file. */
    if (!HMAC_Final(h_ctx, mac, &mac_len))
        goto error;
    if (fwrite(mac, mac_len, 1, out) < 1)
        goto error;

    /* Operation succeded. */
    res = 1;

    /* Cleanup. */
error:
    CLEAR_MLLE_CR_KEY();
    EVP_CIPHER_CTX_free(c_ctx);
    HMAC_CTX_free(h_ctx);
    free(mac);
    free(iv);

    return res;
}
