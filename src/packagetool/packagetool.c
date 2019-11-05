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

/*
* package the library. 
*
* Returns:
*      0 - packaging was successful.
*      >0 - error during packaging.
*/
int doWork(int argc, char **argv)
{
    char *libPath = NULL;
    int success = 0;

    //check if help was requested
    if (helpRequested(argc, argv)) {
        printArgumentHelp();
        return 0;
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

    if (!success) { return 1; }

    printf("Start packing library.\n");
    mlle_log = stderr;
    // --------------------
    // Copy source folder.
    // --------------------
    success = copyFolderStructure();
    if (!success) { return 2 ; }
    // ----------------------------
    // Create the .library folder.
    // ----------------------------
    success = createLibraryFolder();
    if (!success) { return 3; }
    // --------------------------------------------
    // Locate the icon file in the source folder.
    // --------------------------------------------
    success = prepareIconFile();
    if (!success) { return 4 ; }

    // -------------------------------------------
    // Copy LVE's (if needed) to .library folder.
    // -------------------------------------------
    success = copyLVE();
    if (!success) { return 5; }


    // -------------------------------------------
    // Copy Extra files if .library folder exists next to packagetool.
    // -------------------------------------------
    success = copyExtraFiles();
    if (!success) { return 6; }

    // ------------------------------
    // Create the manifest.xml file.
    // ------------------------------
    success = createManifestFile();
    if (!success) { return 7; }

    // ---------------------------------
    // Encrypt .mo-files to .moc-files.
    // ---------------------------------
    success = encryptFiles();
    if (!success) { return 8; }

    // --------------------------
    // Create a zipped archive.
    // --------------------------
    success = createZipArchive();
    if (!success) { return 9; }

    // -------------------------------------
    // Delete the temporary source folder.
    // -------------------------------------
    success = deleteTemporaryStagingFolder();
    if (!success) { 
        printf("Ignoring failure to remove temporary files (%s)\n", getCopiedSourcePath());
        return 0; 
    }

    if (success)
    {
        printf("Library is done.\n");
    }

    cleanUp();
    return 0;
}


int main(int argc, char **argv)
{
    int argSize = 0;
    int ret = 0;
    char errorMsg[200] = {'\0'};
    char *path = NULL;

    if (argc == 1)
    {
        printf("Too few arguments.\n");
        printf("For usage use -h or --help");
        return 1;
    }

    /* OpenSSL initialization stuff. */
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    ret = doWork(argc, argv);

    /* OpenSSL cleanup stuff. */
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();

    return ret;
}



