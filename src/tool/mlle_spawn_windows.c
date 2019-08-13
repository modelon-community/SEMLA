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

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <errno.h>
#include "mlle_types.h"
#include "mlle_error.h"
#include "mlle_spawn.h"

#define   READ_FD 0
#define   WRITE_FD 1


/********************************************************************
 * Setup pipes and start up connection to LVE (server) on Linux.
 * 
 * We are setting up both FILE and Pipes for communication.
 * The FILE streams are however not in use. They worked when
 * OpenSSL was not used but when the streams was connected to BIO
 * descriptors and a SSL handshake was initiated it just froze.
 * With pipes it worked ok.
 *******************************************************************/
struct mlle_connections *
mlle_spawn(const char *exec_name,
           struct mlle_error **error)
{
    //int flags;
#define LONG_FILE_NAME_MAX 4096
    char long_name[LONG_FILE_NAME_MAX] = "\\\\?\\";
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    int status = 0;
    int stdin_open = 1;
    int stdout_open = 1;
    char buffer[250];
    errno_t error_flag;
    int parent_to_child_pipe_fd[2] = { -1, -1 };
    int child_to_parent_pipe_fd[2] = { -1, -1 };
    int tmp_fd_for_stdin_stream = -1;
    int tmp_fd_for_stdout_stream = -1;
    intptr_t child_handle = 0;
    struct mlle_connections *connections = NULL;

    // New. Don't change anything.
    setvbuf( stdin, NULL, _IONBF, 0 );
    setvbuf( stdout, NULL, _IONBF, 0 );

    fflush(NULL);
    status = _pipe(parent_to_child_pipe_fd, BUFSIZ, _O_BINARY | _O_NOINHERIT);
    if (status == -1) {
        mlle_error_set_literal(error, 1, 1, "_pipe() failed.");
        return NULL;
    }

    status = _pipe(child_to_parent_pipe_fd, BUFSIZ, _O_BINARY | _O_NOINHERIT);
    if (status == -1) {
        mlle_error_set_literal(error, 1, 1, "_pipe() failed.");
        return NULL;
    }

    /* Make new file descriptors (FDs) for stdin and stdout streams (so they
     * won't close during the next step).
     */
    tmp_fd_for_stdin_stream = _dup(stdin_fileno);
    if (tmp_fd_for_stdin_stream == -1) {
        _get_errno(&error_flag);
        if (error_flag == EBADF) {
            stdin_open = 0;
        } else {
            sprintf(buffer,"Failed to duplicate STDIN stream (errno: %d).", error_flag);
            mlle_error_set_literal(error, 1, 1, buffer);
            return NULL;
        }
    }
    tmp_fd_for_stdout_stream = _dup(stdout_fileno);
    if (tmp_fd_for_stdout_stream == -1) {
        _get_errno(&error_flag);
        if (error_flag == EBADF) {
            stdout_open = 0;
        } else {
            sprintf(buffer,"Failed to duplicate STDOUT stream (errno: %d).", error_flag);
            mlle_error_set_literal(error, 1, 1, buffer);
            return NULL;
        }
    }

    /* Associate the FDs of the ends of the pipes that the child will use with
     * the FDs of stdin and stdout. Reading from stdin and writing to stdout
     * will now read and write from and to the pipes. The original file streams
     * for stdin and stdout are still open, because we made new FDs for them.
     */
    status = _dup2(parent_to_child_pipe_fd[PIPE_READ_INDEX], stdin_fileno);
    if (status == -1) {
        _get_errno(&error_flag);
        sprintf(buffer,"Failed to duplicate (and close) STDIN stream (errno: %d).", error_flag);
        mlle_error_set_literal(error, 1, 1, buffer);
        return NULL;
    }
    status = _dup2(child_to_parent_pipe_fd[PIPE_WRITE_INDEX], stdout_fileno);
    if (status == -1) {
        _get_errno(&error_flag);
        sprintf(buffer,"Failed to duplicate (and close) STDOUT stream (errno: %d).", error_flag);
        mlle_error_set_literal(error, 1, 1, buffer);
        return NULL;
    }
    /* Close the ends of the pipe that this process won't use. */
    _close(parent_to_child_pipe_fd[PIPE_READ_INDEX]);
    _close(child_to_parent_pipe_fd[PIPE_WRITE_INDEX]);

    /* Start new process. */
    child_handle = _spawnl(_P_NOWAIT, exec_name, exec_name, (char *) NULL);
    if (child_handle == -1) {    
        char errbuf[100];
        size_t exe_name_len = strlen(exec_name);
        strerror_s(errbuf, sizeof(errbuf), errno);
        if (exe_name_len > 255) {
            /* For deep dirs retry with\\?\ prefix and only backward slashes*/
            if (exe_name_len < LONG_FILE_NAME_MAX - 5) {
                char* ch = long_name+4;
                strncpy(long_name + 4, exec_name, LONG_FILE_NAME_MAX - 5);
                while (*ch) {
                    if (*ch == '/') *ch = '\\';
                    ch++;
                }
            }

            child_handle = _spawnl(_P_NOWAIT, long_name, long_name, (char *)NULL);
            if (child_handle == -1) {
                mlle_error_set(error, 1, 1, "Failed to start the process (%s). Potentially due to too long executable file name (%s)", errbuf, exec_name);
                return NULL;
            }
        }
        else {
            mlle_error_set(error, 1, 1, "Failed to create a new process (%s).", errbuf);
            return NULL;
        }
    }

    /* Reassociate the usual FD numbers for stdin and stdout with the original
     * stdin and stdout file streams.
     */
    if (stdin_open) {
        status = _dup2(tmp_fd_for_stdin_stream, stdin_fileno);
        if (status == -1) {
            _get_errno(&error_flag);
            sprintf(buffer,"Failed to reassociate STDIN stream (errno: %d).", error_flag);
            mlle_error_set_literal(error, 1, 1, buffer);
            return NULL;
        }
        
        /* Close the temporary FDs for stdin and stdout. */
        _close(tmp_fd_for_stdin_stream);
    }
    
    if (stdout_open) {
        status = _dup2(tmp_fd_for_stdout_stream, stdout_fileno);
        if (status == -1) {
            _get_errno(&error_flag);
            sprintf(buffer,"Failed to reassociate STDOUT stream (errno: %d).", error_flag);
            mlle_error_set_literal(error, 1, 1, buffer);
            return NULL;
        }
        
        /* Close the temporary FDs for stdin and stdout. */
        _close(tmp_fd_for_stdout_stream);
    }

    connections = calloc(1, sizeof(*connections));
    if (connections != NULL) {
        connections->fd_to_child = parent_to_child_pipe_fd[PIPE_WRITE_INDEX];
        connections->fd_from_child = child_to_parent_pipe_fd[PIPE_READ_INDEX];
    } else {
        mlle_error_set_literal(error, 1, 1, "Out of memory.");
    }

    return connections;
}
