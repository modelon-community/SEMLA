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

#include "libcrypto-compat.h"

#define _XOPEN_SOURCE 700

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "mlle_cr_crypt.h"
#include "random_key_file.h"
#include "mlle_cr_decrypt.h"
#include "mlle_cr_context.h"
#include "mlle_portability.h"
#include "mlle_io.h"
#include "mlle_error.h"


mlle_cr_context* mlle_cr_create(const char* basedir) {
    size_t len = strlen(basedir);
    mlle_cr_context * c = calloc(1, sizeof(mlle_cr_context) + len);
    if (NULL == c) return NULL;
    memcpy(c->basedir, basedir, len);
    return c;
}

void mlle_cr_free(mlle_cr_context* context) {
    free(context);
}

#if defined(DEBUG) || defined(_DEBUG)
void mlle_debug_log_key(const unsigned char* key) {
    int i;
    char buf[MLLE_CR_KEY_LEN * 3 + 4];
    char* ptr = buf;

    for ( i = 0; i < MLLE_CR_KEY_LEN; i++)
    {
        const char* format = (i > 0)? ":%02X" :"%02X";
        unsigned int code = key[i];
        sprintf(ptr, format, code);
        ptr += (i == 0) ? 2 : 3;
        *ptr = '\0';
    }

    fprintf(mlle_log, "key: %s\n", buf);
    buf[0] = 0;
}
#else
void mlle_debug_log_key(const unsigned char* key) { }
#endif


int mlle_demask_key(mlle_cr_context* context, const char* rel_file_path, unsigned char* key) {
    char path[MLLE_LONG_FILE_NAME_MAX];
    char fullpath[MLLE_LONG_FILE_NAME_MAX];
    size_t rel_path_len;
    int last_slash_index = 0;
    int i = 0;
    char ch;
    struct mlle_key_mask_map* key_mask_map = context->keymask_map;
    struct mlle_key_mask_map* map_item = NULL;
    size_t file_size = 0;
    char *file_buffer = NULL;
    char *out_buffer = NULL;
    int ret = 0;
    struct mlle_error * error = 0;

    if (mlle_log) {
        fprintf(mlle_log, "mlle_demask_key: %s\n", rel_file_path);
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
    path[last_slash_index] = 0;

    /* check if this is a package.moc file */
    if (strcasecmp("package.moc", &(rel_file_path[rel_path_len - PACKAGE_MOC_STRLEN])) == 0) {
        if (last_slash_index == 0) {
            /* top level does not have any mask */
            return 1; /* request storing the key after decryption */
        }
        else {
            /* Get a mask from parent dir */
            if (mlle_demask_key(context, path, key) < 0) {
                return -1;
            }
            else {
                return 1;
            }
        }
    }

    /* check if we have a key in cache */
    if(last_slash_index)
        HASH_FIND_STR(key_mask_map, path, map_item);
    else
        HASH_FIND_STR(key_mask_map, "/", map_item);
    if (map_item != NULL) {
       int i;
       for (i = 0; i < MLLE_CR_KEY_LEN; ++i) {
            key[i] = key[i] ^ map_item->key_mask[i];
       }
       if (mlle_log) {
           fprintf(mlle_log, "mlle_demask_key: applied mask for %s\n", rel_file_path);
           mlle_debug_log_key(key);
       }

       return 0;
    }  

    /* if there is package.moc in this directory read key from it; otherwize return parent */
    if (last_slash_index)
        memcpy(path + last_slash_index, "/package.moc", 13); /* strlen("/package.moc") + 1 = 13 */
    else
        memcpy(path + last_slash_index, "package.moc", 12); /* strlen("package.moc") + 1 = 12 */

    snprintf(fullpath, sizeof(fullpath), "%s/%s", context->basedir, path);
    file_buffer = mlle_io_read_file(fullpath, &file_size, &error);
    if (error) {
        /* assume that no file exists; read mask from parent */
        ret = mlle_demask_key(context, path, key);
    }
    else {
        out_buffer = malloc(file_size);
        if (file_buffer && out_buffer) {
            int ret_code = mlle_cr_decrypt(context, path, file_buffer, file_size, out_buffer);
            if (ret_code > 0) {
                /* key mask is now in the hash but it's also the tail in the outbuffer */
                for (i = 0; i < MLLE_CR_KEY_LEN; ++i) {
                    key[i] = key[i] ^ out_buffer[ret_code + i];
                }
                if (mlle_log) {
                    fprintf(mlle_log, "mlle_demask_key: applied mask for %s\n", rel_file_path);
                    mlle_debug_log_key(key);
                }

            }
            else {
                ret = -1;
                goto cleanup;
            }
        }
        else {
            ret = -1;
            goto cleanup;
        }
    }

  cleanup:
    free(file_buffer);
    free(out_buffer);
    mlle_error_free(&error);
    return ret;
}


int mlle_store_keymask(mlle_cr_context* context, const char* rel_file_path, const char* key_mask) {
    struct mlle_key_mask_map* map_item;
    size_t rel_file_path_len = strlen(rel_file_path);
    size_t rel_path_len = (rel_file_path_len == PACKAGE_MOC_STRLEN) ? 0 :rel_file_path_len - (PACKAGE_MOC_STRLEN + 1); /* take out "/package.moc"  */
    char relpath[MLLE_LONG_FILE_NAME_MAX];
    struct mlle_key_mask_map* key_mask_map = context->keymask_map;

    if (rel_path_len) {
        memcpy(relpath, rel_file_path, rel_path_len);
        relpath[rel_path_len] = 0;
    }
    else {
        relpath[0] = '/';
        relpath[1] = '\0';
        rel_path_len = 1;
    }
    HASH_FIND_STR(key_mask_map, relpath, map_item);
    if (map_item) {
        return 0; /* key from this file is already saved in the cache, no need to reread*/
    }

    map_item = (struct mlle_key_mask_map*)calloc(1, sizeof(struct mlle_key_mask_map) + rel_path_len + 1);
    if (map_item == NULL) {
        return -1;
    }
    map_item->relpath = map_item->buffer;
    memcpy(map_item->relpath, relpath, rel_path_len);
    memcpy(map_item->key_mask, key_mask, MLLE_CR_KEY_LEN);

    HASH_ADD_KEYPTR(hh, key_mask_map, map_item->relpath, rel_path_len, map_item);  
    context->keymask_map = key_mask_map;

    if (mlle_log) {
        fprintf(mlle_log, "mlle_store_keymask: stored key_mask for %s \n", rel_file_path);
        mlle_debug_log_key(key_mask);
    }

    return 0;
}


/*
* Decrypt the data pointed to by in to out, where in_len is the length of the data pointed to by in.
*  - key_cache - contains the table of key masks to be used in different directories.
*  - rel_file_path - relative path to the file within library used for finding the needed key_mask
*
* The data pointed to by in is assumed to consist of IV, encrypted mask + data and HMAC, in that order.
* The buffer pointed to by out must be large enough to hold the masc + the decrypted data, which is always shorter than in_len.
* The in and out buffers may not be the same.
*
* Returns the length of the decrypted data, or -1 on an error.
*/
int mlle_cr_decrypt(
    mlle_cr_context* context,
    const char* rel_file_path,
    char* in,
    size_t in_len,
    char* out)
{
    /* TODO: Enable better error messages. */
    EVP_CIPHER_CTX *c_ctx;
    HMAC_CTX *h_ctx;
    unsigned char *iv_in;
    unsigned char *enc_in;
    unsigned char *mac_in;
    unsigned char *out_u = (unsigned char*) out;
    unsigned char *mac = NULL;
    const EVP_CIPHER *cipher;
    int iv_len;
    const EVP_MD *hash;
    unsigned int mac_len;
    int enc_len = 0;
    int out_len = 0;
    int dec_len = 0;
    int res = -1;
    size_t rel_file_path_len = 0;
    int restore_mask_flag = 0;
    DECLARE_MLLE_CR_KEY();

    /* Init structures. */
    c_ctx = EVP_CIPHER_CTX_new(); 
    h_ctx = HMAC_CTX_new(); 
    EVP_CIPHER_CTX_init(c_ctx);
    HMAC_CTX_init(h_ctx);

    /* Select algorithms and get parameters. */
    cipher = MLLE_CR_CIPHER;
    iv_len = EVP_CIPHER_iv_length(cipher);
    hash = MLLE_CR_HASH;
    mac_len = (unsigned int) EVP_MD_size(hash);

    /* Split incoming data. */
    iv_in   = (unsigned char*) in;
    if (in) {
        enc_in = iv_in + iv_len;
        mac_in = iv_in + in_len - mac_len;
        enc_len = (int)(in_len - iv_len - mac_len);

        /* Set up decryption and HMAC calculation. */
        INITIALIZE_MLLE_CR_KEY();
    }
    if (context && rel_file_path) {
        restore_mask_flag = mlle_demask_key(context, rel_file_path, MLLE_CR_KEY);
    }
    if (!in) return 0;

    if (!EVP_DecryptInit_ex(c_ctx, cipher, NULL, MLLE_CR_KEY, iv_in))
        goto error;
    mac = (unsigned char*) malloc(mac_len);
    if (!mac)
        goto error;
    if (!HMAC_Init_ex(h_ctx, MLLE_CR_KEY, MLLE_CR_KEY_LEN, hash, NULL))
        goto error;
    if (!HMAC_Update(h_ctx, iv_in, iv_len))
        goto error;

    /* Decrypt data. */
    if (!EVP_DecryptUpdate(c_ctx, out_u, &dec_len, enc_in, enc_len))
        goto error;
    out_len = dec_len;
    if (!EVP_DecryptFinal_ex(c_ctx, out_u + out_len, &dec_len))
        goto error;
    out_len += dec_len;

    /* Calculate and check HMAC. */
    if (!HMAC_Update(h_ctx, out_u, out_len))
        goto error;
    if (!HMAC_Final(h_ctx, mac, &mac_len))
        goto error;
    if (memcmp(mac, mac_in, mac_len))
        goto error;    

    /* check if this is a package.moc file and save the key into cache */
    if (restore_mask_flag) {
        out_len -= MLLE_CR_KEY_LEN; /* take out key length from the data sent back*/
        if (mlle_store_keymask(context, rel_file_path, out + out_len) < 0)
            goto error;
    }

    res = out_len;
    /* Cleanup. */
error:
    CLEAR_MLLE_CR_KEY();
    EVP_CIPHER_CTX_cleanup(c_ctx);
    EVP_CIPHER_CTX_free(c_ctx);
    HMAC_CTX_free(h_ctx);
    free(mac);

    return res;
}
