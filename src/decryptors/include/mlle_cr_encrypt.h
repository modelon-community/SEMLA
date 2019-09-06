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

#ifndef MLLE_CR_ENCRYPT_H_
#define MLLE_CR_ENCRYPT_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct mlle_cr_context mlle_cr_context;

/*
 * Read data from stream in until eof, encrypt it, and write IV, encrypted store_mask, data and HMAC (in that order) to stream out.
 * key_masc is a xor masc applied to the key (NULL for no masc)
 *
 * Returns zero on error.
 */
int mlle_cr_encrypt(mlle_cr_context* context,
                    const char* relpath,
                    FILE* in,
                    FILE* out);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MLLE_CR_ENCRYPT_H_ */
