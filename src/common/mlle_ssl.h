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

#ifndef PUBLIC_SRC_COMMON_MLLE_SSL_H_
#define PUBLIC_SRC_COMMON_MLLE_SSL_H_

#include "mlle_types.h"
#include <openssl/ssl.h>
#include <openssl/err.h>


#define SERVER 0
#define CLIENT 1

#define SSL_ERROR_BUF_LEN 100

// Forward declarations to avoid compiler warnings.
struct mlle_lve_ctx;
struct mlle_error;

// Storage of the public key for the Tool
// which we receive in the LVE callback method.
extern char *global_tool_pub_key;

/*****************************
 * Initiate the SSL library.
 *****************************/
void init_ssl();


/***************************************************************
 * Read the private/public file and store the information in
 * a RSA structure.
 *
 * Returns:
 *      RSA structure containing information, NULL otherwise.
 ***************************************************************/
RSA* get_rsa(char *private_key);

/********************************************************************
 * Get the public key from the RSA structure.
 *
 * Returns:
 *      A char array with the public key or NULL if unsuccessful.
 *******************************************************************/
char* get_public_key(RSA *rsa);


/************************************************************
 * Generate a X509 certificate.
 * This certificate is self-signed and only used internally
 * by the server-side(LVE).
 *
 * Parameters:
 *      rsa - an RSA strukture with private/public keys.
 *
 * Returns:
 *      A X509 certificate or NULL if something failed.
 ***********************************************************/
X509 *generate_X509(RSA *rsa);


/****************************************************************
 * Initiate the CTX structure that is used by SSL.
 *
 * Parameters:
 *      private_key - the private key to use.
 *      is_client - 1 = create CTX structure for client(Tool).
 *                - 0 = create CTX structure for server(LVE).
 *
 * Returns:
 *      A CTX structure or NULL if something went wrong.
 ***************************************************************/
SSL_CTX* create_CTX(char *private_key, int is_client);


/********************************************************
 * Write a message using SSL.
 *
 * Parameters:
 *      ssl - the SSL structure with I/O information.
 *      message - the message to write.
 *      len - the length of the data
 *
 * Returns:
 *      Number of bytes written or -1 if write failed.
 ********************************************************/
int ssl_write_message(SSL *ssl, char *message, size_t len);


/************************************************************
 * Read a message using SSL.
 *
 * Parameters:
 *      ssl - the SSL structure with I/O information.
 *      messageBuffer - pointer to an uninitialized buffer.
 *
 * Returns:
 *      Number of bytes read (i.e. the message length).
 *      -1 if read failed.
 ***********************************************************/
int ssl_read_message(SSL *ssl, char **messageBuffer);

/*******************************************************
 * Get type of error and error reason as string.
 *
 * Parameters:
 *      errorCode - the integer value of the error.
 *      error - output array with error message.
 * Returns:
 *      Error message.
 ******************************************************/
void ssl_get_error_string(int errorCode, char *error, size_t error_len);


#endif /* PUBLIC_SRC_COMMON_MLLE_SSL_H_ */
