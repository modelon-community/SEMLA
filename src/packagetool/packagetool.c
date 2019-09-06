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
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "arguments.h"
#include "manifest.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

extern FILE* mlle_log;
void doWork(int argc, char **argv)
{
    char *libPath = NULL;
    int success = 0;

    //check if help was requested
    if (helpRequested(argc, argv)) {
        printArgumentHelp();
        return;
    }

    printf("\nValidating arguments.\n");

    // --------------------------------
    // Validate and extract arguments.
    // --------------------------------
    if (validateArguments(argc, argv))
    {
        // Validate the arguments data.
        success = ((validateMandatoryArguments()) &&
                (validateLibraryPath()) &&
                (validateEncryption()) &&
                (validateXmlFile(ARGUMENT_TOOLS_FILE)) &&
                (validateXmlFile(ARGUMENT_DEPENDENCIES_FILE)) &&
                (validateIcon()) );

    }

    if (!success) { return; }

    printf("Start packing library.\n");
    mlle_log = stderr;
    // --------------------
    // Copy source folder.
    // --------------------
    success = copyFolderStructure();
    if (!success) { return; }
    // ----------------------------
    // Create the .library folder.
    // ----------------------------
    success = createLibraryFolder();
    if (!success) { return; }
    // --------------------------------------------
    // Locate the icon file in the source folder.
    // --------------------------------------------
    success = prepareIconFile();
    if (!success) { return; }

    // -------------------------------------------
    // Copy LVE's (if needed) to .library folder.
    // -------------------------------------------
    success = copyLVE();
    if (!success) { return; }

    // ------------------------------
    // Create the manifest.xml file.
    // ------------------------------
    success = createManifestFile();
    if (!success) { return; }

    // ---------------------------------
    // Encrypt .mo-files to .moc-files.
    // ---------------------------------
    success = encryptFiles();
    if (!success) { return; }

    // --------------------------
    // Create a zipped archive.
    // --------------------------
    success = createZipArchive();
    if (!success) { return; }

    // -------------------------------------
    // Delete the temporary source folder.
    // -------------------------------------
    success = deleteTemporaryStagingFolder();
    if (!success) {return; }

    if (success)
    {
        printf("Library is done.\n");
    }

    cleanUp();
}


int main(int argc, char **argv)
{
    int argSize = 0;
    char errorMsg[200] = {'\0'};
    char *path = NULL;

    if (argc == 1)
    {
        printf("Too few arguments.\n");
        printf("For usage use -h or --help");
        return 0;
    }

    /* OpenSSL initialization stuff. */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    doWork(argc, argv);

    /* OpenSSL cleanup stuff. */
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();

    return 0;
}



