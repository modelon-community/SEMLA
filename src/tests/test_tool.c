/*
Copyright (C) 2015-2019 Modelon AB

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

#include "test_tool.h"

#define N_TEST_FILES 3 
const char *FACIT_FILES[N_TEST_FILES]     = { "package.mo",  "Module/package.mo", "binary.gif" };
const char *FILES_ENCRYPTED[N_TEST_FILES] = { "package.moc", "Module/package.moc", "binary.gif" };

void print_usage() {
    printf(
"USAGE: test_tool [options]\n"
" The code runs a unit tests suit and returns 0 status code on success.\n"
" The unit tests are:\n"
"   - start LVE and establish secure connection\n"
"   - checks that protocol version is 1\n"
"   - set encrypted library path\n"
"   - checkout a licensed feature\n"
"   - try checkout a non-licensed feature\n"
"   - for each test file request it and verify content with reference\n"
"\nOptions:\n"
"--lve <lve_name>   the name of the lve to use (default: lve_linux64).\n"
"--libpath <path>   path (either relative from current directory or absolute)\n"
"                     for the encrypted library (default: test_library).\n"
"--reflibpath <path>  path for the unencrypted library with reference files\n"
"                    (default: test_facit).\n"
"--file <ecrypted_file> <reference_file> encrypted file to request and file to compare to\n"
"                   (default will test 3 files: package.moc,Module/package.moc,binary.gif).\n"
"--feature <name>   license feature to checkout that is expected to work.\n"
"                   (default: test_licensed_feature).\n"
"--no-feature <name>   license feature to try to checkout that is expected to fail.\n"
"                   (default: test_not_licensed_feature).\n"
"--help              print usage and exit. This must be the only option given.\n"
    );
}

int main(int argc, char **argv)
{
    char lve_name[100] = "lve_linux64";
    char library_path[10000] = "test_library";
    char reflib_path[10000] = "test_facit";
    
    char feature[100] = "test_licensed_feature";
    char no_feature[100] = "test_non_licensed_feature";

    int n_test_files = N_TEST_FILES;
    char encrypted_file[1000];
    char ref_file[1000];
    const char**  reference = FACIT_FILES;
    const char** encrypted_files = FILES_ENCRYPTED;

    int i;

    if(1 == argc){
        print_usage();
        exit(1);
    }
    
    for (i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--lve")) {
            i++;
            snprintf(lve_name, sizeof(lve_name), "%s", argv[i]);
        }
        else if (0 == strcmp(argv[i], "--libpath")) {
            i++;
#ifdef _WIN32
            snprintf(library_path, sizeof(library_path), "\\\\?\\%s", argv[i]);
#else
            snprintf(library_path, sizeof(library_path), "%s", argv[i]);
#endif
        }
        else if (0 == strcmp(argv[i], "--reflibpath")) {
            i++;
            snprintf(library_path, sizeof(library_path), "%s", argv[i]);
        }
        else if (0 == strcmp(argv[i], "--file")) {
            i++;
            snprintf(encrypted_file, sizeof(encrypted_file), "%s", argv[i]);
            i++;
            snprintf(ref_file, sizeof(ref_file), "%s", argv[i]);
        }
        else if (0 == strcmp(argv[i], "--feature")) {
            i++;
            snprintf(feature, sizeof(feature), "%s", argv[i]);
        }
        else if (0 == strcmp(argv[i], "--no-feature")) {
            i++;
            snprintf(feature, sizeof(no_feature), "%s", argv[i]);
        }
        else if (0 == strcmp(argv[i], "--help")) {
            print_usage();
            exit(argc == 2);
        }
        else {
            printf("ERROR: Unexpected options. Use 'test_tool --help' for usage.\n");
            exit(1);
        }
    }
    if (i != argc) {
        print_usage();
        printf("\nERROR: Could not parse command line args. Use 'test_tool --help' for usage.\n");
        exit(1);
    }

    printf("#test_tool: Info: using lve: '%s'.\n", lve_name) ;

    test_lib(
        lve_name,
        feature,
        no_feature,
        n_test_files,
        library_path, encrypted_files,
        reflib_path, reference
        ) ;

    i = display_check_statistics();
    return i;
}

void test_lib(
const char * lve_name, 
const char* feature,
const char* no_feature,
int number_of_files,
const char * library_path, const char **library_files,
const char * facit_path, const char **facit_files
)
{
    struct mlle_connections *lve = NULL;
    struct mlle_error *error = NULL;

    char lve_path[1024] ;
    snprintf(lve_path, sizeof(lve_path), "%s/.library/%s", library_path, lve_name);

#ifdef WIN32
    {
        char* ch = lve_path;
        while (*ch) {
            if (*ch == '/') *ch = '\\';
            ch++;
        }
    }
#endif

    lve = mlle_start_executable(lve_path, &error);

    check_mlle(lve != NULL, "connect to  LVE", &error);
    mlle_error_free(&error);

    if (lve) {
        int i = -1;
        char test_name[1024];

        check_mlle(mlle_tool_version(lve, 1, 1, &error), "Test protocol version [1, 1]", &error);
        mlle_error_free(&error);

        snprintf(test_name, sizeof(test_name), "Test set library path ('%s')", library_path);
        check_mlle(mlle_tool_libpath(lve, library_path, &error), test_name, &error);
        mlle_error_free(&error);

        snprintf(test_name, sizeof(test_name), "Test valid feature ('%s')", feature);
        check_mlle(mlle_tool_feature(lve, feature, &error), test_name, &error);
        mlle_error_free(&error);

        snprintf(test_name, sizeof(test_name), "Test invalid feature ('%s')", no_feature);
        check_mlle(!mlle_tool_feature(lve, no_feature, &error), test_name, &error);
        mlle_error_free(&error);

        for (i = 0; i < number_of_files; i++) {
            get_file_and_compare(library_files[i], facit_files[i], facit_path, lve);
        }

        mlle_connections_free(&lve);
        }
}

void get_file_and_compare(const char* get_file,
                         const char* correct_file,
                         const char* lib_path,
                         struct mlle_connections *lve)
{
    struct mlle_error *error = NULL; 
    struct mlle_file_contents *file = NULL;

    FILE *correct;
    size_t size_file = 0;
    size_t size_correct = 0;

    char *file_buf = NULL;
    char *correct_buf = NULL;
    char correct_path[PATH_MAX+1];

    int equals;
    int i = 0;

    char test_name[1000];
    char test_error[1000];

    snprintf (test_name, sizeof(test_name), "FILE('%s')", get_file);

    file = mlle_tool_file(lve, get_file, &error);
    check_mlle(file != NULL, test_name, &error);
    mlle_error_free(&error);
        
    if (file) {
        // Get contents from LVE
        size_file = mlle_tool_get_file_size(file);
        file_buf = malloc(size_file);
        mlle_tool_read_bytes(file, file_buf, size_file);
        mlle_file_contents_free(&file);

        // Read facit file
        snprintf(correct_path, PATH_MAX, "%s/%s", lib_path, correct_file);
        correct = fopen(correct_path, "rb");
        
        snprintf (test_error, sizeof(test_error), "Could not open file '%s'", correct_path);
        snprintf (test_name, sizeof(test_name), "open facit ('%s')", correct_path);
        check(correct != NULL, test_name, test_error);
            
        if (correct == NULL)
            return;

        fseek(correct, 0, SEEK_END);
        size_correct = ftell(correct);
        correct_buf = malloc(size_correct);
        fseek(correct, 0, SEEK_SET);
        fread(correct_buf, 1, size_correct, correct);
        fclose(correct);

        check(size_file == size_correct, "sizes match", "");

        equals = 1;
        for (i = 0; equals && (size_t) i < size_file; i++) {
            equals = file_buf[i] == correct_buf[i];
        }

        check(equals, "identical contents", "");

        free(file_buf);
        free(correct_buf);
    }
}

void check_mlle(int success, char *test_name, struct mlle_error **error)
{
    if(NULL != *error)
        check(success, test_name, mlle_error_get_message(*error));
    else
        check(success, test_name, NULL);
}

static int test_run = 0 ;
static int test_ok  = 0 ;

void check(int success, char *test_name, char *error)
{
    test_run++ ;
    
    if(NULL == test_name){
        test_name = "test not named" ;
    }

    if (success) {
        fprintf(stderr, "ok %d - %s\n", test_run, test_name);
        test_ok++;
    } else if (error) {
        fprintf(stderr, "nok %d - %s\n", test_run, test_name);
        fprintf(stderr, "# %s\n", error);
    } else {
        fprintf(stderr, "nok %d - %s\n", test_run, test_name);
        fprintf(stderr, "# no error message!\n");
    }
}

int display_check_statistics(void)
{
    fprintf(stderr,"\ntests: %d, ok: %d, fail: %d\n", test_run, test_ok, test_run - test_ok);

    return test_run - test_ok ;    
}




