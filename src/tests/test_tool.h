#ifndef TEST_TOOL_H
#define TEST_TOOL_H

#define _XOPEN_SOURCE 700

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "mlle_licensing.h"
#include "mlle_portability.h"
#include "mlle_ssl.h"
#include "mlle_types.h"

#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

void check_mlle(int success, char *test_name, struct mlle_error **error) ;
void check(int success, char *test_name, char *error) ;
int display_check_statistics(void) ;

void get_file_and_compare(const char* get_file,
                         const char* correct_file,
                         const char* lib_path,
                         struct mlle_connections *lve) ;

void test_lib(
    const char * lve_name,
    const char* feature,
    const char* no_feature,
    int number_of_files,
    const char * library_path, const char **library_files,
    const char * facit_path, const char **facit_files
    );

#endif

