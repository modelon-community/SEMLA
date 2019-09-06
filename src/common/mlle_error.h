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


#ifndef MLLE_ERROR_H_
#define MLLE_ERROR_H_


#define _XOPEN_SOURCE 700
#include <stdarg.h>
#define MLLE_LONG_FILE_NAME_MAX 4096

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct mlle_error;

struct mlle_error *
mlle_error_new(int domain,
               int code,
               const char *format,
               ...);

struct mlle_error *
mlle_error_new_valist(int domain,
                      int code,
                      const char *format,
                      va_list args);

struct mlle_error *
mlle_error_new_literal(int domain,
                       int code,
                       const char *message);

void
mlle_error_set(struct mlle_error **error,
               int domain,
               int code,
               const char *format,
               ...);

void
mlle_error_set_literal(struct mlle_error **error,
                       int domain,
                       int code,
                       const char *message);

void
mlle_error_free(struct mlle_error **error);

void
mlle_error_propagate(struct mlle_error **dest,
                     struct mlle_error *src);

int
mlle_error_get_domain(struct mlle_error *error);

int
mlle_error_get_code(struct mlle_error *error);

struct mlle_error *
mlle_error_get_cause(struct mlle_error *error);

char *
mlle_error_get_message(struct mlle_error *error);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MLLE_ERROR_H_ */
