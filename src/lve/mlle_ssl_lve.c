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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ossl_typ.h>
#include <openssl/asn1t.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>

#include "mlle_ssl.h"
#include "mlle_error.h"
#include "mlle_ssl_lve.h"

#include "private_key_lve.h"



/**************************************************************
 * This method catches the certificate the client sends.
 * We are only interested in the public key from the client
 * so we extract the key and tell the server that the
 * certificate is valid (i.e. return 1).
 *
 * Parameters:
 *      ctx - the holder for the certificate.
 *      arg - arguments. Not in use here.
 *
 * Returns:
 *      1 - public key was extracted with success.
 *      0 - failed to extract key (handshake will fail).
 *************************************************************/
static int cert_verify_callback(X509_STORE_CTX *ctx, void *arg)
{
    EVP_PKEY *public_key = NULL;
    X509 *x509 = NULL;
    RSA *rsa = NULL;

    // Get the certificate.


    x509 = X509_STORE_CTX_get0_cert(ctx);
    if (x509 == NULL)
    {
            return 0;
    }

    // Get public key with DER encoding.
    if ( (public_key = X509_get_pubkey(x509)) == NULL)
    {
        return 0;
    }

    // Get RSA with public key.
    if ( (rsa = EVP_PKEY_get1_RSA(public_key)) == NULL)
    {
        return 0;
    }

    // Extract public key in PEM format and store it globally.
    if ( (global_tool_pub_key = get_public_key(rsa)) == NULL)
    {
        return 0;
    }

    RSA_free(rsa);
    EVP_PKEY_free(public_key);

    return 1;
}


/************************************************************
 * Setup the SSL structure for the LVE (server).
 * The structure is using a callback method so when the
 * handshake take part we can get hold of the tool
 * certificate and its public key.
 *
 * Parameters:
 *      lve_ctx - I/O and information about the connection.
 *
 * Returns:
 *      1 - the setup was successful.
 *      0 - setup failed.
 ***********************************************************/
int ssl_setup_lve(struct mlle_lve_ctx *lve_ctx)
{
    SSL *ssl = NULL;
    SSL_CTX* ctx = NULL;
    BIO *bioWrite = NULL;
    BIO *bioRead = NULL;
    RSA *rsa = NULL;
    X509 *x509;
    int result = 0;
    DECLARE_PRIVATE_KEY_LVE();

    // Initiate ssl.
    init_ssl();

    // Initiate CTX structure.
    INITIALIZE_PRIVATE_KEY_LVE();
    if ( (ctx = create_CTX((char *) PRIVATE_KEY_LVE, SERVER)) == NULL)
    {
        lve_ctx->tool_error_type = MLLE_PROTOCOL_SSL_ERROR;
        lve_ctx->tool_error_msg = "SSL: Failed to create server CTX structure.";
        goto cleanup;
    }

    // Create RSA with private key.
    if ( (rsa = get_rsa((char *) PRIVATE_KEY_LVE)) == NULL)
    {
        lve_ctx->tool_error_type = MLLE_PROTOCOL_SSL_ERROR;
        lve_ctx->tool_error_msg = "SSL: Failed to create server RSA structure.";
        goto cleanup;
    }
    CLEAR_PRIVATE_KEY_LVE();

    // Add callback method so we can get hold of the Tool certificate.
    SSL_CTX_set_cert_verify_callback (ctx, cert_verify_callback, NULL);

    // Setup the SSL structure.
    ssl = SSL_new(ctx);

    // Setup input/output using file descriptors.
    bioWrite = BIO_new_fp(lve_ctx->out_stream, BIO_NOCLOSE);
    bioRead = BIO_new_fp(lve_ctx->in_stream, BIO_NOCLOSE);

    if ( (bioWrite == NULL) || (bioRead == NULL) )
    {
        lve_ctx->tool_error_type = MLLE_PROTOCOL_SSL_ERROR;
        lve_ctx->tool_error_msg = "SSL: Failed to create server BIO input/output descriptors.";
        goto cleanup;
    }

    // Set to non-blocking(1). 0 = blocking.
    BIO_set_nbio(bioWrite, 1);
    BIO_set_nbio(bioRead, 1);

    // Create self-signed certificate.
    if ( (x509 = generate_X509(rsa)) == NULL)
    {
        lve_ctx->tool_error_type = MLLE_PROTOCOL_SSL_ERROR;
        lve_ctx->tool_error_msg = "SSL: Failed to create server X509 certificate.";
        goto cleanup;
    }

    // Setup the SSL structure.
    ssl = SSL_new(ctx);

    // Connect SSL with read and write BIO.
    SSL_set_bio(ssl, bioRead, bioWrite );

    // Add certificate to SSL.
    if (SSL_use_certificate(ssl, x509) == 0)
    {
        lve_ctx->tool_error_type = MLLE_PROTOCOL_SSL_ERROR;
        lve_ctx->tool_error_msg = "SSL: Failed to add X509 certificate.";
        goto cleanup;
    }

    // Do any handshakes in the background.
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // Disable read ahead for SSL_pending.
    SSL_set_read_ahead(ssl, 0);

    lve_ctx->ssl = ssl;

    result = 1;

cleanup:
    CLEAR_PRIVATE_KEY_LVE();

    return result;
}


/**********************************************************
 * LVE (server) performs a TLS handshake and trying to
 * connect with the Tool (client) using SSL.
 *
 * Parameters:
 *      lve_ctx - I/O and information about the connection.
 *
 * Returns:
 *      1 - handshake was successful.
 *      0 - handshake failed.
 **********************************************************/
int lve_perform_handshake(struct mlle_lve_ctx *lve_ctx)
{
    int result = 0;
    int errorCode = 0;
    char errorString[SSL_ERROR_BUF_LEN];

    if ( (lve_ctx == NULL) || (lve_ctx->ssl == NULL) )
    {
        return 0;
    }
    
    // Wait for Tool (client) to connect.
    if ( (result = SSL_accept(lve_ctx->ssl)) <= 0)
    {
        // Get reason for handshake error.
        errorCode = SSL_get_error(lve_ctx->ssl, result);
        ssl_get_error_string(errorCode, errorString, SSL_ERROR_BUF_LEN);
        lve_ctx->tool_error_type = MLLE_PROTOCOL_SSL_ERROR;
        lve_ctx->tool_error_msg = errorString;

        return 0;
    }

    return 1;
}
