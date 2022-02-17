#
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
 
cmake_minimum_required(VERSION 2.8)

# ===========================================
# This is the makefile for the unit tests.
# ===========================================

# Flags
# If this option is on you will get a message of type
# "LINK : warning LNK4098: defaultlib 'MSVCRTD' conflicts with use of other libs; use /NODEFAULTLIB:library".
#et(CMAKE_BUILD_TYPE RELEASE)


enable_testing()

# ------------------------------------------------------------
# Add path where find_library method can look for libraries.
# ------------------------------------------------------------
if (WIN32)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Program\ Files\ (x86)/check/lib")
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/LibraryEncryption/Build/openssl/lib")
else()
    # Path to libcrypto.a and libssl.a.
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/lib/x86_64-linux-gnu/")
endif()


if (WIN32)
    set(CHECK_INSTALL_DIR "C:/Program\ Files\ (x86)/check")    
    include_directories(${CHECK_INSTALL_DIR}/include)
else()
    include_directories("/usr/include/")    # Path to check.h and check_stdint.h.
endif()

MESSAGE(STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})


# -----------------------------
# Find libraries for OpenSSL.
# -----------------------------
if (WIN32)
    find_library(CHECK_LIBRARY NAMES check ) #PATHS "C:/Program\ Files\ (x86)/check")
    find_library(COMPAT_LIBRARY NAMES compat ) #PATHS "C:/Program\ Files\ (x86)/check/")
    
    find_library(CRYPTO libeay32)       # Crypt library.
    find_library(OPEN_SSL ssleay32)     # OpenSSL library.
else()
    set(EXTRA_LIBS m rt pthread dl)
    
    find_library(CRYPTO libcrypto.a)    # Crypt library.
    find_library(OPEN_SSL libssl.a)     # OpenSSL library.
    find_library(CHECK_LIBRARY libcheck.a)  # Check library.
endif()

MESSAGE( STATUS "CRYPTO: " ${CRYPTO} )
MESSAGE( STATUS "CHECK: " ${CHECK_LIBRARY} )

# Libeay32 depends on ws2_32.lib.
if(WIN32)
    # Values for Windows
    if(BITS64)
        find_library(WINSOCK_LIB NAMES ws2_32 PATH "C:/Program\ Files\ (x86)/Windows\ Kits/8.1/Lib/winv6.3/um/x64")
    else() 
        find_library(WINSOCK_LIB NAMES ws2_32 PATH "C:/Program\ Files\ (x86)/Windows\ Kits/8.1/Lib/winv6.3/um/x86")
    endif()            
endif()


# Add libs to one variable.
if (WIN32)
    set(LIBS ${LIBS} ${CHECK_LIBRARY} ${COMPAT_LIBRARY})
else()
    set(LIBS ${LIBS} ${CHECK_LIBRARY} )
endif()

include_directories(../) 



# ----------------------
# Set path to OpenSSL.
# ----------------------

if(WIN32)
    # Windows.
    set(OPENSSL_DIR "C:/LibraryEncryption/Build/openssl/include") 
    include_directories(${OPENSSL_DIR})
else()
    # Linux
    set(OPENSSL_DIR "/home/johan/Modelon/build/openssl/include/")
    include_directories(${OPENSSL_DIR})
endif()
  

add_library(tool
    ../tool/mlle_licensing.c 
    ../common/mlle_protocol.c 
    ../common/mlle_error.c 
    ../common/mlle_io.c 
    ../common/mlle_parse_command.c
    ../common/mlle_utils.c
    ../common/mlle_ssl.c    
    ${CMAKE_CURRENT_BINARY_DIR}/private_key_tool.c
)

add_library(lve
    ../lve/lve.c 
    ../lve/mlle_lve.c 
    ../common/mlle_protocol.c 
    ../lve/mlle_protocol_lve_state.c 
    ../common/mlle_error.c 
    ../common/mlle_io.c 
    ../common/mlle_parse_command.c 
    ../lve/mlle_lve_tools.c 
    ../lve/mlle_lve_libpath.c 
    ../lve/mlle_lve_pubkey.c 
    ../lve/mlle_lve_license.c 
    ../lve/mlle_lve_feature.c 
    ${CMAKE_CURRENT_BINARY_DIR}/public_key_tool.c
    ${CMAKE_CURRENT_BINARY_DIR}/private_key_lve.c    
)


add_executable(test_main test_main.c)

if (WIN32)
    target_link_libraries(test_main tool lve ${LIBS} ${OPEN_SSL} ${CRYPTO} ${WINSOCK_LIB})
else()
    target_link_libraries(test_main tool lve ${LIBS} ${OPEN_SSL} ${CRYPTO} ${EXTRA_LIBS})
endif()

add_test(test_main test_main)


