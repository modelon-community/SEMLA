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

#define _XOPEN_SOURCE 700
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/* libcrypto-compat.h must be first */
#include "libcrypto-compat.h"
#include "mlle_io.h"
#include "mlle_lve.h"


#ifdef _MSC_VER

/*
 * If we compile using MSVC we have to use this main method signature to
 * prevent annoying command prompt pop ups when starting process from a
 * process with a GUI.
 */
int main() {
    return WinMain(0, 0,0, 0);
}

int
WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)

#else

int
main(void)

#endif

{
    struct mlle_lve_ctx lve_ctx = { stdin, stdout, NULL, NULL, 0, 0, 0, NULL, NULL, NULL };

    mlle_log_open("SEMLA_LVE_LOG_FILE");

    mlle_lve_init(&lve_ctx);

    // Set upp SSL.
    if (ssl_setup_lve(&lve_ctx))
    {
        // Connect with Tool (client).
        if (lve_perform_handshake(&lve_ctx))
        {
            // Start receiving data.
            mlle_lve_receive(&lve_ctx);
        }
    }

    mlle_lve_shutdown(&lve_ctx);

    return EXIT_SUCCESS;
}
