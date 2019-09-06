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

#ifndef MLLE_CR_DECRYPT_H_
#define MLLE_CR_DECRYPT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct mlle_cr_context mlle_cr_context;

/*
* Allocate decryptor context object to be passed in all encrypt/decrypt operations.
*/
mlle_cr_context* mlle_cr_create(const char* basedir);

/*
* Free up decryptor/encryptor context.
*/
void mlle_cr_free(mlle_cr_context* context);

/*
 * Decrypt the data pointed to by in to out, where in_len is the length of the data pointed to by in.
 *  context - pointer to the structure allocated by mlle_cr_create
 *  relpath - pointer to the file to be processed for subdir depedent encryption.
 * The data pointed to by in is assumed to consist of IV, encrypted mask + data and HMAC, in that order.
 * The buffer pointed to by out must be large enough to hold the decrypted mask+data, which is always shorter than in_len.
 * The in and out buffers may not be the same.
 *
 * Returns the length of the decrypted data, or -1 on an error.
 */
int mlle_cr_decrypt(mlle_cr_context* context, 
                    const char* relpath,
                    char* in,
                    size_t in_len,
                    char* out);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MLLE_CR_DECRYPT_H_ */
