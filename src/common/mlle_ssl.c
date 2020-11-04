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

// Disable "deprecated" warning.
#ifdef WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"

#include <openssl/ossl_typ.h>
#include <openssl/asn1t.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>

#include "mlle_lve.h"
#include "mlle_ssl.h"
#include "mlle_error.h"

char *global_tool_pub_key;

/*****************************
 * Initiate the SSL library.
 *****************************/
void init_ssl()
{
    // Initiate SSL library.
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms(); 
}


/****************************************************************
 * Initiate the CTX structure that is used by SSL.
 *
 * Parameters:
 *      private_key - the private key to use.
 *      mode - 1 => create CTX structure for client(Tool).
 *             0 => create CTX structure for server(LVE).
 *
 * Returns:
 *      A CTX structure or NULL if something went wrong.
 ***************************************************************/
SSL_CTX* create_CTX(char *private_key, int mode)
{
    SSL_CTX* ctx = NULL;
    RSA *rsa = NULL;

    // Initiate the CTX structure.
    if (mode == CLIENT)
    {
        // Setup as client.
        if ( (ctx = SSL_CTX_new( SSLv23_client_method())) == NULL)
        {
            return 0;
        }

        // Client (Tool) will receive a certificate from the
        // server (LVE) but will not verify it.
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    }
    else
    {
        // Setup as server
        if ( (ctx = SSL_CTX_new( SSLv23_server_method())) == NULL)
        {
            return 0;
        }

        // The server(LVE) sends a client certificate request to the
        // client(TOOL) and optional the client can send a certificate
        // in return.
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    }

    // Tell which type of ciphers to use.
    if (SSL_CTX_set_cipher_list(ctx, "HIGH:!DSS:!aNULL@STRENGTH") == 0)
    {
        // Error setting ciphers to use.
        return 0;
    }

    // Create RSA containing the private key.
    if ( (rsa = get_rsa(private_key)) == NULL)
    {
        return 0;
    }

    // Connect private key to CTX.
    if (SSL_CTX_use_RSAPrivateKey(ctx, rsa) == 0)
    {
        return 0;
    }

    return ctx;
}


/*********************************************************************
 * Read the private/public file and store the
 * information in a RSA structure.
 *
 * Parameters:
 *      private_key -the array with key data. This array
 *                   contains both the private and the public key.
 *
 * Returns:
 *      An RSA structure, NULL otherwise.
 ********************************************************************/
RSA* get_rsa(char *private_key)
{
    RSA *rsa = NULL;
    BIO *bio = NULL;

    // Create a memory BIO.
    if ( (bio = BIO_new_mem_buf(private_key, -1)) == NULL)
    {
        return 0;
    }

    // Read private key from memory to RSA.
    rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);

    BIO_free(bio);

    return rsa;
}


/********************************************************************
 * Extract the public key from an RSA structure to a char array.
 * The key is extracted in PEM format (base64 encoded data).
 * The extracted key will start with -----BEGIN PUBLIC KEY-----.
 *
 * Returns:
 *      An array containing the public key if extraction was
 *      successful. NULL otherwise.
 *******************************************************************/
char* get_public_key(RSA *rsa)
{
    BIO *mem = NULL;
    int key_length = 0;
    char *public_key = NULL;

    // Validate parameter.
    if (rsa == NULL)
    {
        return 0;
    }

    // Create a memory BIO.
    if ( (mem = BIO_new(BIO_s_mem())) == NULL)
    {
        return 0;
    }

    // Write public key from RSA structure to memory.
    if (PEM_write_bio_RSA_PUBKEY(mem, rsa))
    {
        key_length = BIO_pending(mem);
        public_key = (char*)malloc(key_length + 1);

        // Read from memory to array.
        BIO_read(mem, public_key, key_length);
        public_key[key_length] = '\0';
    }

    BIO_free(mem);
    return public_key;
}



/**************************************************************
 * Generate a self-signed X509 certificate.
 *
 * Parameters:
 *      rsa - an RSA structure containing private/public keys.
 *
 * Returns:
 *      A X509 certificate or NULL if something failed.
 *************************************************************/
X509 *generate_X509(RSA *rsa)
{
    X509 *x509 = NULL;
    EVP_PKEY * pkey = NULL;
    X509_NAME * name = NULL;

    // Validate input.
    if (rsa == NULL)
    {
        return NULL;
    }

    // Copy RSA to EVP_KEY.
    pkey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pkey, rsa);

    x509 = X509_new();

    // Serial number of certificate set to 1.
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

    // Set expiration date on certificate.
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);  // One year.

    // Set the public key.
    X509_set_pubkey(x509, pkey);

    name = X509_get_subject_name(x509);

    // Set the country code and common name.
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)"SE", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)"Modelon", -1, -1, 0);

    // Set the issuer.
    X509_set_issuer_name(x509, name);

    // Sign the certificate with our key.
    if ( !X509_sign(x509, pkey, EVP_sha1()) )
    {
        return NULL;
    }

    return x509;
}


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
int ssl_write_message(SSL *ssl, char *message, size_t len)
{
    int bytes = 0;
    int errorCode = 0;
    char error[SSL_ERROR_BUF_LEN];

    while ((bytes = SSL_write(ssl, message, (int)len)) < 0)
    {
        errorCode = SSL_get_error(ssl, bytes);        

        // If we get a SSL_ERROR_WANT_WRITE error then the
        // write operation must be repeated with the same arguments.
        if (errorCode != SSL_ERROR_WANT_WRITE) {
            ssl_get_error_string(errorCode, error, SSL_ERROR_BUF_LEN);
            break;
        }
    }

    // If write is successful we get number of bytes written as result.
    if (bytes <= 0)
    {
        return -1;
    }

    // Must flush here otherwise the message will not be sent properly.
    fflush(NULL);

    return bytes;
}


/************************************************************
 * Read a message using SSL.
 *
 * Parameters:
 *      ssl - the SSL structure with I/O information.
 *      messageBuffer - pointer to an initialized buffer of
 *                      size 16 kb (i.e. an SSL record).
 *
 * Returns:
 *      Number of bytes read or -1 if error occurred.
 ***********************************************************/
int ssl_read_message(SSL *ssl, char **messageBuffer)
{
    int noOfBytes = 0;
    int errorCode = 0;
    int pendingData = 0;
    int keepReading = 1;
    int neededSpace = 0;
    char *buffer = NULL;
    char *tmpPointer = NULL;
    int bufferLen = 0;
    char tmpBuffer[MSG_SIZE] = {0};
    int totalBytesRead = 0;
    int errorResult = 0;
    
    while(keepReading)
    {
        // Read data. Number of bytes read is returned.
        noOfBytes = SSL_read(ssl, &tmpBuffer, MSG_SIZE - 1);

        // Did reading fail?
        if (noOfBytes <= 0)
        {
            // Some error has occurred.
            errorCode = SSL_get_error(ssl, noOfBytes);

            switch(errorCode)
            {
                case SSL_ERROR_WANT_READ:
                    errorResult = 0;
                    break;
                case SSL_ERROR_WANT_WRITE:
                    errorResult = 0;
                    break;
                case SSL_ERROR_SYSCALL:
                    errorResult = -1;
                    break;
                case SSL_ERROR_SSL:
                    errorResult = -1;
                    break;
                default:
                    errorResult = -1;
                    break;
            }

            if (errorResult == 0)
            {
                // Call ssl_read again if errorcode is
                // want_read or want_write call.
                continue;
            }
            else
            {
                // Client has shutdown, I/O error or similar.
                return errorResult;
            }
        }

        // Any bytes left in the SSL buffer.
        pendingData = SSL_pending(ssl);

        // Check if message buffer has room for data
        neededSpace = totalBytesRead + noOfBytes + pendingData;
        if (neededSpace > bufferLen)
        {
            // Save any old message buffer
            tmpPointer = buffer;

            // Allocate message buffer
            buffer = malloc(neededSpace);
            if (buffer == NULL)
            {
                return -1;
            }

            // Handle any old data
            if (tmpPointer != NULL)
            {
                // Copy
                memcpy(buffer, tmpPointer, totalBytesRead);
                // Clear old message buffer
                memset(tmpPointer, 0, bufferLen);
                // Free old message buffer
                free(tmpPointer);
                tmpPointer = NULL;
            }
        }

        // Copy data
        memcpy(buffer + totalBytesRead, tmpBuffer, noOfBytes);
        totalBytesRead += noOfBytes;

        // Check if there is more to read
        keepReading = SSL_want_read(ssl) || SSL_pending(ssl);
    }

    // Clear read buffer.
    memset(tmpBuffer, 0, sizeof(tmpBuffer));

    // Pass back pointer to message buffer.
    *messageBuffer = buffer;

    return totalBytesRead;
}


/*******************************************************
 * Get type of error and reason for error.
 *
 * Parameters:
 *      error_code - the integer value of the error
 *      error      - output array that will recieve error message
 *      error_len  - length of output array
 * Returns:
 *      Error message.
 ******************************************************/
void ssl_get_error_string(int error_code, char *error, size_t error_len)
{
    unsigned long error_value;
    const char* err_str = "";

    switch(error_code)
    {
        case SSL_ERROR_NONE:
            break;
        case SSL_ERROR_WANT_READ:
            err_str = "SSL_ERROR_WANT_READ";
            break;
        case SSL_ERROR_WANT_WRITE:
            err_str = "SSL_ERROR_WANT_WRITE";
            break;
        case SSL_ERROR_SYSCALL:
            err_str = "SSL_ERROR_SYSCALL";
            break;
        case SSL_ERROR_SSL:
            err_str = "SSL_ERROR_SSL";
            break;
    }

    while ((error_value = ERR_get_error()) > 0)
    {
        snprintf(error, error_len, "%s. reason: %s", err_str, (char*) ERR_reason_error_string(error_value));
    }
}
