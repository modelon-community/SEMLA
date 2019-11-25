#    Copyright (C) 2015 Modelon AB
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the BSD style license.
#
#     This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    BSD_License.txt file for more details.
#
#    You should have received a copy of the BSD_License.txt file
#    along with this program. If not, contact Modelon AB <http://www.modelon.com>.
#

#set(CMAKE_VERBOSE_MAKEFILE ON)

# -------------------------------------------------------------------
# This script is used for generation of public_key_tool.h file.
# -------------------------------------------------------------------

# required defines:
# PUBLIC_KEY_TOOL_NUM number of public keys available
# PUBLIC_KEY_TOOL_H name of the output file to generate

message(STATUS "Generating ${PUBLIC_KEY_TOOL_H} for ${PUBLIC_KEY_TOOL_NUM} keys")
math(EXPR MAXIND ${PUBLIC_KEY_TOOL_NUM}-1)
# generate number of keys define
FILE(WRITE "${PUBLIC_KEY_TOOL_H}"
"#define PUBLIC_KEY_TOOL_NUM ( ${PUBLIC_KEY_TOOL_NUM} )
"
)
# generate includes for all keys
foreach(PUBLIC_KEY_TOOL_I RANGE ${MAXIND})
  FILE(APPEND "${PUBLIC_KEY_TOOL_H}"
"#include \"public_key_tool${PUBLIC_KEY_TOOL_I}.h\"
"
)
endforeach()

# generate DECLARE_PUBLIC_KEY_TOOL()
FILE(APPEND "${PUBLIC_KEY_TOOL_H}"
"#define DECLARE_PUBLIC_KEY_TOOL()"
)
foreach(PUBLIC_KEY_TOOL_I RANGE ${MAXIND})
  FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "DECLARE_PUBLIC_KEY_TOOL${PUBLIC_KEY_TOOL_I}();"
)
endforeach()

FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "unsigned char* PUBLIC_KEY_TOOL[]={"
)
foreach(PUBLIC_KEY_TOOL_I RANGE ${MAXIND})
  FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "PUBLIC_KEY_TOOL${PUBLIC_KEY_TOOL_I},"
)
endforeach()

FILE(APPEND "${PUBLIC_KEY_TOOL_H}" 
"0}
")

  
# generate DECLARE_PUBLIC_KEY_TOOL_LEN()
FILE(APPEND "${PUBLIC_KEY_TOOL_H}"
"#define DECLARE_PUBLIC_KEY_TOOL_LEN() size_t PUBLIC_KEY_TOOL_LEN[]={"
)
foreach(PUBLIC_KEY_TOOL_I RANGE ${MAXIND})
  FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "PUBLIC_KEY_TOOL${PUBLIC_KEY_TOOL_I}_LEN,"
)
endforeach()
FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "0}
")

# generate INITIALIZE_PUBLIC_KEY_TOOL()
FILE(APPEND "${PUBLIC_KEY_TOOL_H}"
"#define INITIALIZE_PUBLIC_KEY_TOOL() do {"
)
foreach(PUBLIC_KEY_TOOL_I RANGE ${MAXIND})
  FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "INITIALIZE_PUBLIC_KEY_TOOL${PUBLIC_KEY_TOOL_I}();"
)
endforeach()
FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "} while (0)
")

# generate CLEAR_PUBLIC_KEY_TOOL()
FILE(APPEND "${PUBLIC_KEY_TOOL_H}"
"#define CLEAR_PUBLIC_KEY_TOOL() do {"
)
foreach(PUBLIC_KEY_TOOL_I RANGE ${MAXIND})
  FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "CLEAR_PUBLIC_KEY_TOOL${PUBLIC_KEY_TOOL_I}();"
)
endforeach()
FILE(APPEND "${PUBLIC_KEY_TOOL_H}" "} while (0)
")
