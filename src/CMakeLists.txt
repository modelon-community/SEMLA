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
# This is the make-file for the public version of LibraryEncryption.
# -------------------------------------------------------------------

cmake_minimum_required(VERSION 3.4)
project(MLLE)

set(USE_OPENSSL_10_API FALSE)

if (EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    message(STATUS "Will rely on OpenSSL conan package")
    set(USE_CONAN_OPENSSL TRUE)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
    message(STATUS "CMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}" )
    add_custom_target(openssl DEPENDS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
else()
    include(ExternalProject)
    set(USE_CONAN_OPENSSL FALSE)
endif()

    set(CMAKE_BUILD_TYPE Release)
    if(WIN32 AND NOT DYNAMIC)
        foreach(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")
        endforeach(flag_var)
    endif(WIN32 AND NOT DYNAMIC)

# Propagate value of GLOBAL_LICENSE_FEATURE to build
if(DEFINED GLOBAL_LICENSE_FEATURE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMLLE_GLOBAL_LICENSE_FEATURE=${GLOBAL_LICENSE_FEATURE}")
endif(DEFINED GLOBAL_LICENSE_FEATURE)

# Platforms
# - Bitness
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8" OR (WIN32 AND CMAKE_GENERATOR MATCHES "Win64$"))
    message(STATUS "Target is 64 bits")
    set(BITS64 1)
    set(BITS 64)
else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    message(STATUS "Target is 32 bits")
    set(BITS64 0)
    set(BITS 32)
endif()


if (WIN32)
    set(LVETARGET "lve_win${BITS}")
else()
    set(LVETARGET "lve_linux${BITS}")
endif()

# the following commands are needed to fix a problem with the libraries
# for linux 64 bits
if(NOT WIN32)
    message(STATUS "x86_64 architecture detected - setting flag -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fPIC")
endif()

if (USE_CONAN_OPENSSL)
    message(STATUS "CONAN_OPENSSL_ROOT=${CONAN_OPENSSL_ROOT}, CONAN_LIB_DIRS_OPENSSL=${CONAN_LIB_DIRS_OPENSSL}")
    message(STATUS "CONAN_LIBS_OPENSSL=${CONAN_LIBS_OPENSSL}, CONAN_LIBS=${CONAN_LIBS}")
    set(openssl_dir ${CONAN_OPENSSL_ROOT})
    set(ssl_libs ${CONAN_LIBS} )
else()
# - OS
if(WIN32)
    # Values for Windows
    if(BITS64)
        if (MSVC)
            set(openssl_conf_cmd    perl Configure VC-WIN64A)
            set(openssl_conf_extra  perl configdata.pm --dump)
        else()
            set(openssl_conf_cmd    perl Configure mingw64)
            set(openssl_conf_extra  make depend)
        endif()
    else()
        if (MSVC)
            set(openssl_conf_cmd    perl Configure no-shared no-idea no-mdc2 no-rc5 --openssldir=. --prefix=${CMAKE_CURRENT_BINARY_DIR}/openssl VC-WIN32 )
            set(openssl_conf_extra  perl configdata.pm --dump)
        else()
            set(openssl_conf_cmd    perl Configure mingw)
            set(openssl_conf_extra  make depend)
        endif()
    endif()
    if (MSVC)
        set(openssl_make        nmake)
        set(openssl_crypto_lib  libcrypto.lib)
        set(openssl_ssl_lib     libssl.lib)
    else()
        set(openssl_make        make)
        set(openssl_crypto_lib  libcrypto.a)
        set(openssl_ssl_lib     libssl.a)
    endif()
    set(extra_ssl_libs      crypt32 ws2_32)
    else()
        # Values for Linux, etc
        set(openssl_conf_cmd    ./config shared)
        set(openssl_conf_extra  make depend)
        set(openssl_make        make)
        set(openssl_crypto_lib  libcrypto.a)
        set(openssl_ssl_lib     libssl.a)
        set(extra_ssl_libs      dl pthread)
    endif()

    # - OpenSSL as external project
    if (USE_OPENSSL_10_API)
        message(STATUS "Will build OpenSSL 1.0.2 as external project")

    ExternalProject_Add(openssl
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/openssl
        URL ${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/openssl-1.0.2
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${openssl_conf_cmd} no-shared no-idea no-mdc2 no-rc5 --openssldir=. --prefix=${CMAKE_CURRENT_BINARY_DIR}/openssl
        COMMAND ${openssl_conf_extra}
        BUILD_COMMAND ${openssl_make}
        INSTALL_COMMAND ${openssl_make} install
        )
    else()
        message(STATUS "Will build OpenSSL 1.1.1a as external project")
    ExternalProject_Add(openssl
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/openssl
        URL ${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/openssl-1.1.1a
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${openssl_conf_cmd} no-shared no-idea no-mdc2 no-rc5 --openssldir=. --prefix=${CMAKE_CURRENT_BINARY_DIR}/openssl
        COMMAND ${openssl_conf_extra}
        BUILD_COMMAND ${openssl_make}
        INSTALL_COMMAND ${openssl_make} install
    )
    endif()
    set(openssl_dir ${CMAKE_CURRENT_BINARY_DIR}/openssl)
    add_library(ssl STATIC IMPORTED)
    add_library(crypto STATIC IMPORTED)
    set_property(TARGET ssl PROPERTY IMPORTED_LOCATION ${openssl_dir}/lib/${openssl_ssl_lib})
    set_property(TARGET crypto PROPERTY IMPORTED_LOCATION ${openssl_dir}/lib/${openssl_crypto_lib})
    add_dependencies(ssl openssl)
    add_dependencies(crypto openssl)

    set(ssl_libs ssl crypto)

    include_directories(${CMAKE_CURRENT_BINARY_DIR}/openssl/include )
endif()


if(WIN32)
    set(tool_platform_c     ${CMAKE_CURRENT_LIST_DIR}/tool/mlle_spawn_windows.c)
else()
    set(tool_platform_c     ${CMAKE_CURRENT_LIST_DIR}/tool/mlle_spawn_posix.c)
endif()

# Libraries
message(STATUS "Current binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${CONAN_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_LIST_DIR}/encrypt_decrypt
                    ${CMAKE_CURRENT_LIST_DIR}/decryptors/include
                    ${CMAKE_CURRENT_LIST_DIR}/license_managers/include
                    ${CMAKE_CURRENT_LIST_DIR}/obfuscators/include
                    ${CMAKE_CURRENT_LIST_DIR}/embedfile
                    ${CMAKE_CURRENT_LIST_DIR}/common
                    ${CMAKE_CURRENT_LIST_DIR}/lve
                    ${CMAKE_CURRENT_LIST_DIR}/tool
                    ${CMAKE_CURRENT_BINARY_DIR})


# Compile obfuscate program.
add_executable(obfuscate
               ${CMAKE_CURRENT_LIST_DIR}/embedfile/obfuscate.c
               ${CMAKE_CURRENT_LIST_DIR}/embedfile/obfuscate_utils.c)
target_link_libraries(obfuscate
                      ${ssl_libs} ${extra_ssl_libs}
                      obfuscator_module)


# Compile the randomize key program.
add_executable(randomize_key
               ${CMAKE_CURRENT_LIST_DIR}/embedfile/randomize_key.c
               ${CMAKE_CURRENT_LIST_DIR}/embedfile/obfuscate_utils.c)
target_link_libraries(randomize_key
                      ${ssl_libs}
                      ${extra_ssl_libs}
                      obfuscator_module)

# -------------------------------------------
# Key length to use when generating RSA keys.
# -------------------------------------------
if(NOT DEFINED RSA_KEY_LENGTH)
    set(RSA_KEY_LENGTH 4096)
endif()

# -------------------------------------------------
# Optional directory for the private/public keys.
# -------------------------------------------------
set(KEYS_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/openssl_keys" CACHE STRING "option")

# ------------------------------------------------------------
# Directory to use if keys are missing from source directory.
# ------------------------------------------------------------
set(KEYS_BUILD_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/openssl_keys)

# --------------------------------------
# Use Tool key from source or create a new one.
# --------------------------------------
set(PRIVATE_KEY_TOOL ${KEYS_SOURCE_DIRECTORY}/private_key_tool.pem)
if(EXISTS "${PRIVATE_KEY_TOOL}")
    message("SSL key ${PRIVATE_KEY_TOOL} exists. Using it.")
elseif(EXISTS "${KEYS_BUILD_DIRECTORY}/private_key_tool.pem")
    set(PRIVATE_KEY_TOOL ${KEYS_BUILD_DIRECTORY}/private_key_tool.pem)
    message("SSL key ${PRIVATE_KEY_TOOL} exist in build directory. Using it.")
else()
    message("SSL key ${PRIVATE_KEY_TOOL} doesn't exist. A new key is created.")
    file(MAKE_DIRECTORY "${KEYS_BUILD_DIRECTORY}")
    set(PRIVATE_KEY_TOOL ${KEYS_BUILD_DIRECTORY}/private_key_tool.pem)
    execute_process(COMMAND openssl genrsa -out ${PRIVATE_KEY_TOOL} ${RSA_KEY_LENGTH})
endif()

# -------------------------------------
# Use LVE key from source or create a new one.
# -------------------------------------
set(PRIVATE_KEY_LVE ${KEYS_SOURCE_DIRECTORY}/private_key_lve.pem)
if(EXISTS "${PRIVATE_KEY_LVE}")
    message("SSL key ${PRIVATE_KEY_LVE} exists. Using it.")
elseif(EXISTS "${KEYS_BUILD_DIRECTORY}/private_key_lve.pem")
    set(PRIVATE_KEY_LVE ${KEYS_BUILD_DIRECTORY}/private_key_lve.pem)
    message("SSL key ${PRIVATE_KEY_LVE} exists in build directory. Using it.")
else()
    message("SSL key ${PRIVATE_KEY_LVE} doesn't exist. A new key is created.")
    file(MAKE_DIRECTORY "${KEYS_BUILD_DIRECTORY}")
    set(PRIVATE_KEY_LVE ${KEYS_BUILD_DIRECTORY}/private_key_lve.pem)
    execute_process(COMMAND openssl genrsa -out ${PRIVATE_KEY_LVE} ${RSA_KEY_LENGTH})
endif()


# ---------------------------------------------
# Use public Tool key from source or extract from private.
# ---------------------------------------------
set(PUBLIC_KEY_TOOL ${KEYS_SOURCE_DIRECTORY}/public_key_tool.pem)
if(EXISTS "${PUBLIC_KEY_TOOL}")
    message("SSL key ${PUBLIC_KEY_TOOL} exists. Using it.")
elseif(EXISTS "${KEYS_BUILD_DIRECTORY}/public_key_tool.pem")
    set(PUBLIC_KEY_TOOL ${KEYS_BUILD_DIRECTORY}/public_key_tool.pem)
    message("SSL key ${PUBLIC_KEY_TOOL} exists. Using it.")
else()
    message("SSL key ${PUBLIC_KEY_TOOL} doesn't exist. Public key is extracted from ${PRIVATE_KEY_TOOL}.")
    file(MAKE_DIRECTORY "${KEYS_BUILD_DIRECTORY}")
    set(PUBLIC_KEY_TOOL ${KEYS_BUILD_DIRECTORY}/public_key_tool.pem)
    execute_process(COMMAND openssl rsa -pubout -in "${PRIVATE_KEY_TOOL}" -out "${PUBLIC_KEY_TOOL}")
endif()


# Create header file for tool private key.
set(PRIVATE_KEY_TOOL_H "${CMAKE_CURRENT_BINARY_DIR}/private_key_tool.h")
add_custom_command(
    OUTPUT "${PRIVATE_KEY_TOOL_H}"
    COMMAND "${CMAKE_COMMAND}" -E echo
        Running: obfuscate "${PRIVATE_KEY_TOOL_H}" "${PRIVATE_KEY_TOOL}" PRIVATE_KEY_TOOL LVE_PRIVATE
    COMMAND obfuscate "${PRIVATE_KEY_TOOL_H}" "${PRIVATE_KEY_TOOL}" PRIVATE_KEY_TOOL LVE_PRIVATE
    DEPENDS "${PRIVATE_KEY_TOOL}" obfuscate
    )
set_source_files_properties("${PRIVATE_KEY_TOOL_H}" PROPERTIES HEADER_FILE_ONLY TRUE)

# Create header file for library private key.
set(PRIVATE_KEY_LVE_H "${CMAKE_CURRENT_BINARY_DIR}/private_key_lve.h")
add_custom_command(
    OUTPUT "${PRIVATE_KEY_LVE_H}"
    COMMAND "${CMAKE_COMMAND}" -E echo
        Running: obfuscate "${PRIVATE_KEY_LVE_H}" "${PRIVATE_KEY_LVE}" PRIVATE_KEY_LVE TOOL_PRIVATE
    COMMAND obfuscate "${PRIVATE_KEY_LVE_H}" "${PRIVATE_KEY_LVE}" PRIVATE_KEY_LVE TOOL_PRIVATE
    DEPENDS "${PRIVATE_KEY_LVE}" obfuscate
    )
set_source_files_properties("${PRIVATE_KEY_LVE_H}" PROPERTIES HEADER_FILE_ONLY TRUE)

# Create header file for tool public key.
set(PUBLIC_KEY_TOOL_H "${CMAKE_CURRENT_BINARY_DIR}/public_key_tool.h")
add_custom_command(
    OUTPUT "${PUBLIC_KEY_TOOL_H}"
    COMMAND "${CMAKE_COMMAND}" -E echo
        Running: obfuscate "${PUBLIC_KEY_TOOL_H}" "${PUBLIC_KEY_TOOL}" PUBLIC_KEY_TOOL TOOL_PUBLIC
    COMMAND obfuscate "${PUBLIC_KEY_TOOL_H}" "${PUBLIC_KEY_TOOL}" PUBLIC_KEY_TOOL TOOL_PUBLIC
    DEPENDS "${PUBLIC_KEY_TOOL}" obfuscate
    )
set_source_files_properties("${PUBLIC_KEY_TOOL_H}" PROPERTIES HEADER_FILE_ONLY TRUE)

add_library(error
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_error.c
)

# Dummy output which is never actually produced. Anything that depends on
# this will always be rebuilt.
add_custom_target(always_rebuild)


# Modules
set(MLLE_LIST_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(MLLE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(MLLE_THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/../ThirdParty") # TODO: Need to be included?

# - License manager
if(NOT TEST_LICENSED_FEATURE)
    set(TEST_LICENSED_FEATURE test_licensed_feature)
endif()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/license_managers ${CMAKE_CURRENT_BINARY_DIR}/license_managers)

# - Decryptor
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/decryptors ${CMAKE_CURRENT_BINARY_DIR}/decryptors)

add_dependencies(decryptor  openssl)

# - Obfuscator
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/obfuscators ${CMAKE_CURRENT_BINARY_DIR}/obfuscators)

# --------------
# Create lve.
# --------------
add_executable(${LVETARGET}
    ${PRIVATE_KEY_LVE_H}
    ${PUBLIC_KEY_TOOL_H}
    ${CMAKE_CURRENT_LIST_DIR}/lve/lve.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_error.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_io.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_parse_command.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_protocol.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_ssl.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.h

    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_lve.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_protocol_lve_state.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_lve_feature.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_lve_file.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_lve_libpath.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_lve_license.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_lve_pubkey.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_lve_tools.c
    ${CMAKE_CURRENT_LIST_DIR}/lve/mlle_ssl_lve.c
)

target_link_libraries(${LVETARGET} license_manager decryptor ${ssl_libs} ${extra_ssl_libs})
if(MSVC )
        # add NODEFAULTLIB:libcmt in Debug
        # into string
        set_target_properties(${LVETARGET} PROPERTIES LINK_FLAGS_Debug  "/NODEFAULTLIB:libcmt")
endif()

# --------------------
# Create tool library.
# --------------------
add_library(tool
    ${PRIVATE_KEY_TOOL_H}
    ${CMAKE_CURRENT_LIST_DIR}/tool/mlle_licensing.c
    ${CMAKE_CURRENT_LIST_DIR}/tool/mlle_ssl_tool.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_protocol.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_error.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_io.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_parse_command.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_ssl.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mlle_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.h
    ${tool_platform_c}
)

add_executable(test_tool
    ${CMAKE_CURRENT_LIST_DIR}/tests/test_tool.c
)

target_link_libraries(test_tool tool ${ssl_libs} ${extra_ssl_libs})

file(COPY ${CMAKE_CURRENT_LIST_DIR}/tool/mlle_licensing.h
          ${CMAKE_CURRENT_LIST_DIR}/tool/mlle_types.h
          ${CMAKE_CURRENT_LIST_DIR}/common/mlle_error.h
     DESTINATION include)

# --------------------
# Create packagetool.
# --------------------
add_executable(packagetool
    ${CMAKE_CURRENT_LIST_DIR}/packagetool/packagetool.c
    ${CMAKE_CURRENT_LIST_DIR}/packagetool/arguments.c
    ${CMAKE_CURRENT_LIST_DIR}/packagetool/manifest.c
    ${CMAKE_CURRENT_LIST_DIR}/packagetool/utils.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.h
)

target_link_libraries(packagetool ${ssl_libs} ${extra_ssl_libs} decryptor)
# --------------------
# Create encrypt_file.
# --------------------
add_executable(encrypt_file
    ${CMAKE_CURRENT_LIST_DIR}/encrypt_decrypt/encrypt_file.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.c
)
target_link_libraries(encrypt_file ${ssl_libs} ${extra_ssl_libs} decryptor)

# --------------------
# Create decrypt_file.
# --------------------
add_executable(decrypt_file
    ${CMAKE_CURRENT_LIST_DIR}/encrypt_decrypt/decrypt_file.c
    ${CMAKE_CURRENT_LIST_DIR}/common/libcrypto-compat.c
)
target_link_libraries(decrypt_file ${ssl_libs} ${extra_ssl_libs} decryptor)


if(WIN32)
    if (MSVC)
        # Setting /SUBSYSTEM:WINDOWS to force build with WinMain and avoid command window pop-up at start
        set_target_properties(${LVETARGET} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")

        # ignore missing pdb warning for openssl
        set_target_properties(${LVETARGET} PROPERTIES LINK_FLAGS "/ignore:4099")
        set_target_properties(obfuscate PROPERTIES LINK_FLAGS "/ignore:4099")
        set_target_properties(randomize_key PROPERTIES LINK_FLAGS "/ignore:4099")
        set_target_properties(decrypt_file PROPERTIES LINK_FLAGS "/ignore:4099")
        set_target_properties(packagetool PROPERTIES LINK_FLAGS "/ignore:4099")
        set_target_properties(encrypt_file PROPERTIES LINK_FLAGS "/ignore:4099")
        set_target_properties(test_tool PROPERTIES LINK_FLAGS "/ignore:4099")
        set_target_properties(tool PROPERTIES LINK_FLAGS "/ignore:4099")
    endif (MSVC)
endif(WIN32)

add_custom_command( TARGET ${LVETARGET}
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E remove_directory $<TARGET_FILE_DIR:packagetool>/LVE
        COMMAND "${CMAKE_COMMAND}" -E make_directory $<TARGET_FILE_DIR:packagetool>/LVE
        COMMAND "${CMAKE_COMMAND}" -E touch ${CMAKE_CURRENT_BINARY_DIR}/test_library.mol
        COMMAND "${CMAKE_COMMAND}" -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/test_library.mol
        COMMAND "${CMAKE_COMMAND}" -E echo
        "Copying: $<TARGET_FILE:${LVETARGET}> to $<TARGET_FILE_DIR:packagetool>/LVE"
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:${LVETARGET}> $<TARGET_FILE_DIR:packagetool>/LVE/
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_custom_target(test_facit
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_library
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_library/package.mo
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_library/binary.gif
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_library/Module/package.mo
    COMMAND "${CMAKE_COMMAND}" -E echo "Copying test library to test_facit"
    COMMAND "${CMAKE_COMMAND}" -E remove_directory test_facit
    COMMAND
      "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/tests/test_library test_facit
    COMMAND
      "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/tests/test_library bin/test_facit
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(test_library_mol
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/test_library.mol
        packagetool
        ${LVETARGET}
        test_facit
        )

set(PACKAGE_TEST_LIB_CMD $<TARGET_FILE:packagetool>
            -librarypath ${CMAKE_CURRENT_LIST_DIR}/tests/test_library -version "2.0" -language "3.2" -encrypt "true")

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/test_library.mol
    COMMAND "${CMAKE_COMMAND}" -E touch ${CMAKE_CURRENT_BINARY_DIR}/test_library.mol
    COMMAND "${CMAKE_COMMAND}" -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/test_library.mol
    COMMAND "${CMAKE_COMMAND}" -E echo
        Running: ${PACKAGE_TEST_LIB_CMD}
    COMMAND
        ${PACKAGE_TEST_LIB_CMD}
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_library/package.mo
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_library/binary.gif
        packagetool
        ${LVETARGET}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT LONG_FOLDER_NAME)
  set(LONG_FOLDER_NAME "X01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789/0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${LONG_FOLDER_NAME})
add_custom_target(test_library_long_path
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/test_library_long_path_marker
        )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/test_library_long_path_marker
    DEPENDS 
        ${CMAKE_CURRENT_BINARY_DIR}/test_library/.library/manifest.xml
    COMMAND "${CMAKE_COMMAND}" -E echo "Copying test_library into folder with long name"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory test_library ${LONG_FOLDER_NAME}/test_library
    COMMAND "${CMAKE_COMMAND}" -E touch test_library_mol_marker
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
        )


add_custom_target(
    test_library
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/test_library/.library/manifest.xml
        test_facit
)

if(UNIX)
    set(CHMOD chmod +x)
else()
    set(CHMOD "${CMAKE_COMMAND}" -E echo )
endif()

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/test_library/.library/manifest.xml
    COMMAND "${CMAKE_COMMAND}" -E remove_directory test_library
    COMMAND "${CMAKE_COMMAND}" -E echo "Extracting from test_library.mol"
    COMMAND "${CMAKE_COMMAND}" -E tar xvz test_library.mol
    COMMAND "${CMAKE_COMMAND}" -E copy_directory test_library/Module test_library/${LONG_FOLDER_NAME}    
    COMMAND ${CHMOD} ${CMAKE_CURRENT_BINARY_DIR}/test_library/.library/${LVETARGET}
    DEPENDS
        test_library_mol
        packagetool
        ${LVETARGET}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)


# --------------------
# TESTING
# --------------------

ENABLE_TESTING()

add_test( NAME run_test_tool COMMAND test_tool --lve ${LVETARGET} --feature ${TEST_LICENSED_FEATURE}
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME package_test_lib
    COMMAND "${CMAKE_COMMAND}" --build . --target test_library --config $<CONFIGURATION>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

SET_TESTS_PROPERTIES (
    run_test_tool PROPERTIES DEPENDS package_test_lib)

add_test( NAME run_test_tool_single_file 
         COMMAND test_tool --lve ${LVETARGET}  --feature ${TEST_LICENSED_FEATURE}
                --libpath ${CMAKE_CURRENT_BINARY_DIR}/test_library 
                --file ${LONG_FOLDER_NAME}/package.moc Module/package.mo
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

SET_TESTS_PROPERTIES (
    run_test_tool_single_file PROPERTIES DEPENDS package_test_lib)

add_test( NAME run_test_tool_long_path 
          COMMAND test_tool --lve ${LVETARGET} --feature ${TEST_LICENSED_FEATURE}
            --libpath ${CMAKE_CURRENT_BINARY_DIR}/${LONG_FOLDER_NAME}/test_library
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME package_test_lib_long_path
    COMMAND "${CMAKE_COMMAND}" --build . --target test_library_long_path --config $<CONFIGURATION>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    
SET_TESTS_PROPERTIES (
    run_test_tool_long_path PROPERTIES DEPENDS package_test_lib_long_path)



# --------------------
# INSTALL
# --------------------
# TODO: Install does not work as intended - needs work or maybe it should just be removed?
install(TARGETS test_tool encrypt_file packagetool
        DESTINATION bin
        ARCHIVE DESTINATION lib)

install(TARGETS ${LVETARGET} DESTINATION bin/LVE)

# Install header files needed when using tool library
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION include)

install(DIRECTORY "${openssl_dir}/include/"
        DESTINATION include)

# install(FILES mlle_licensing.h
        # DESTINATION include)

# Install OpenSSL libs as well, since they are needed by tool lib
if (USE_CONAN_OPENSSL)
else()
    install( FILES
        ${openssl_dir}/lib/${openssl_ssl_lib}
        ${openssl_dir}/lib/${openssl_crypto_lib}
         DESTINATION lib)
endif()

install(TARGETS tool DESTINATION lib)
