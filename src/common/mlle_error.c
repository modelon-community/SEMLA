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

#define _XOPEN_SOURCE 700
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#include "mlle_error.h"

#define DEFAULT_MSG_SIZE (2048)

#ifdef _MSC_VER
#define strdup(s) _strdup(s)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

struct mlle_error {
    int               domain;
    int               code;
    struct mlle_error *cause;
    char              *message;
};



struct mlle_error *
mlle_error_new(int domain,
               int code,
               const char *format,
               ...)
{
    struct mlle_error *error;
    va_list arg_pointer;

    va_start(arg_pointer, format);
    error = mlle_error_new_valist(domain, code, format, arg_pointer);
    va_end(arg_pointer);

    return error;
}

struct mlle_error *
mlle_error_new_valist(int domain,
                      int code,
                      const char *format,
                      va_list args)
{
    struct mlle_error *error;
    char *message;

    error = calloc(1, sizeof(*error));
    if (error == NULL) {
        return NULL;
    }

    error->domain = domain;
    error->code = code;
    error->cause = NULL;

    message = calloc(DEFAULT_MSG_SIZE, 1);
    if (message != NULL) {
        vsnprintf(message, DEFAULT_MSG_SIZE, format, args);
    }
    error->message = message;

    return error;
}

void
mlle_error_set(struct mlle_error **error,
               int domain,
               int code,
               const char *format,
               ...)
{
    if (error != NULL) {
        va_list arg_pointer;

        assert(*error == NULL);
        va_start(arg_pointer, format);
        *error = mlle_error_new_valist(domain, code, format, arg_pointer);
        va_end(arg_pointer);
    }
}

struct mlle_error *
mlle_error_new_literal(int domain,
                       int code,
                       const char *message)
{
    struct mlle_error *error;

    error = calloc(1, sizeof(*error));
    if (error == NULL) {
        return NULL;
    }

    error->domain = domain;
    error->code = code;
    error->cause = NULL;
    if (message != NULL) {
        error->message = strdup(message);
    } else {
        error->message = NULL;
    }

    return error;
}

void
mlle_error_set_literal(struct mlle_error **error,
                       int domain,
                       int code,
                       const char *message)
{
    if (error != NULL) {
        assert(*error == NULL);
        *error = mlle_error_new_literal(domain, code, message);
    }
}

void
mlle_error_free(struct mlle_error **error)
{
    if (error == NULL || *error == NULL) {
        return;
    }

    if ((*error)->cause != NULL) {
        mlle_error_free(&((*error)->cause));
    }

    free((*error)->message);
    free(*error);
    *error = NULL;
}

void
mlle_error_propagate(struct mlle_error **dest,
                     struct mlle_error *src)
{
    if (dest != NULL) {
        assert(*dest == NULL);
        *dest = src;
    } else {
        mlle_error_free(&src);
    }
}

int
mlle_error_get_domain(struct mlle_error *error)
{
    return error->domain;
}

int
mlle_error_get_code(struct mlle_error *error)
{
    return error->code;
}

struct mlle_error *
mlle_error_get_cause(struct mlle_error *error)
{
    return error->cause;
}

char *
mlle_error_get_message(struct mlle_error *error)
{
    return error->message;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
