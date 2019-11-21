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

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

// Common obfuscating methods.
#include "obfuscate_utils.h"
#include "mlle_portability.h"

void read_file(char* file, char** buf, size_t* len);

FILE* open_or_exit(const char* fname, const char* mode);

/***********************************************************************
 * The purpose of this file is to read a ssl-key (in a pem file)
 * and create a header file with macros that can produce the key.
 *
 * Usage: 
 *   obfuscate <output file> <input pem file> <name of variable> <type of key>
 *  where <type of key> can be: TOOL_PUBLIC, TOOL_PRIVATE, or LVE_PRIVATE
 *
 * Example: 
 *   % obfuscate private_key_tool.h private_key.pem PRIVATE_KEY_TOOL TOOL_PRIVATE
 *
 * The example above will generate:
 *   private_key_tool.h
 *
 * The macros in the generated h-file is can be used in a function to access 
 * the key as <name of variable> with size <name of variable>_LEN.
 *
 * Usage in a c-file (assuming file name above):
 *   #import "private_key_tool.h"
 *   DECLARE_PRIVATE_KEY_TOOL();
 *   INITIALIZE_PRIVATE_KEY_TOOL();
 *   <use key in PRIVATE_KEY_TOOL with length PRIVATE_KEY_TOOL_LEN>
 *   CLEAR_PRIVATE_KEY_TOOL();
 *
 * The generated h-file does not include any ifndef/define construct - 
 * do not include in a header file.
 **********************************************************************/
int main(int argc, char **argv)
{
    size_t file_size = 0;
    char *data = NULL;
    char *out_file = NULL;
    char *in_file = NULL;
    char *var_name = NULL;
    key_type type;
    int arg_ok = 1;

    if (argc != 5) {
        arg_ok = 0;
    } else if (strcmp(argv[4], "TOOL_PUBLIC") == 0) {
        type = TOOL_PUBLIC;
    } else if (strcmp(argv[4], "TOOL_PRIVATE") == 0) {
        type = TOOL_PRIVATE;
    } else if (strcmp(argv[4], "LVE_PRIVATE") == 0) {
        type = LVE_PRIVATE;
    } else {
        arg_ok = 0;
    }

    if (!arg_ok) {
        printf("Usage: obfuscate <output file> <input pem file> <name of variable> <type of key>\n"
               "  where <type of key> can be: TOOL_PUBLIC, TOOL_PRIVATE, or LVE_PRIVATE");
        return 1;
    }

    out_file = argv[1];
    in_file = argv[2];
    var_name = argv[3];

    read_file(in_file, &data, &file_size);
    create_header_file(var_name, type, data, file_size, out_file);
    free(data);

    return 0;
}


/****************************************************
 * Read a file and return its size and contents.
 * A new buffer is allocated, and must be freed by caller.
 * 
 * Will exit with error message on failure.
 * 
 * Parameters:
 *      file  - path to file
 *      buf   - pointer to a pointer to receive the address of the buffer
 *      len   - pointer to a variable to receive the length
 ***************************************************/
void read_file(char* file, char** buf, size_t* len) {
    size_t read;
    struct stat fileInfo;
    FILE *fp;
    int staterr;

    // Get size of file.
    staterr = stat(file, &fileInfo);
    if (staterr != 0) {
        fprintf(stderr, "Could not get info on %s\n", *file);
        fprintf(stderr, "ERROR [%d] %s", staterr, strerror(staterr));
        exit(1);
    }
    *len = fileInfo.st_size;

    *buf = (char*) calloc(*len + 1, 1);
    if (*buf == NULL) {
        fprintf(stderr, "Could not allocate %d bytes\n", *len);
        exit(1);
    }

    // Open ssl-key file and read content to array.
    // keys must have linux line endings, this must be checked in the build system
    fp = open_or_exit(file, "rb");
    read = fread(*buf, *len, 1, fp);
    fclose(fp);

    if (read != 1) {
        fprintf(stderr, "Error reading 1 block of %d bytes from file '%s'\nread = %d\n", *len, file, read);
        exit(1);
    }
}
