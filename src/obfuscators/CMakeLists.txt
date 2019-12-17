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
project(Obfuscator)

# Use dummy obfuscator if none is selected
if(NOT DEFINED OBFUSCATOR)
   set(OBFUSCATOR dummy)
endif()



if(EXISTS ${OBFUSCATOR}/src/CMakeLists.txt)
    set(OBFUSCATOR_SRC_FOLDER ${OBFUSCATOR}/src)
    set(OBFUSCATOR_BIN_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/obfuscator)
else()
    set(OBFUSCATOR_SRC_FOLDER ${CMAKE_SOURCE_DIR}/obfuscators/${OBFUSCATOR}/src)
    set(OBFUSCATOR_BIN_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/obfuscators/${OBFUSCATOR}/src)
# Check that given obfuscator exists, else list available ones
if(NOT EXISTS ${OBFUSCATOR_SRC_FOLDER}/CMakeLists.txt)
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${OBFUSCATOR}/src/CMakeLists.txt)
        file(GLOB children RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
        file(GLOB children2 RELATIVE ${CMAKE_SOURCE_DIR}/obfuscators ${CMAKE_SOURCE_DIR}/obfuscators/*)
        set(children "${children};${children2}")
        set(dirlist "")
        set(found 0)
        foreach(child ${children})
            if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${child}/src/CMakeLists.txt)
                set(dirlist "${dirlist} ${child}")
                set(found 1)
            elseif(EXISTS ${CMAKE_SOURCE_DIR}/obfuscators/${child}/src/CMakeLists.txt)
                set(dirlist "${dirlist} ${child}")
                set(found 1)
            endif()
        endforeach()
        if(found)
            message(FATAL_ERROR "No such obfuscator: ${OBFUSCATOR}
    Available obfuscators are: ${dirlist}")
        else()
            message(FATAL_ERROR "No obfuscators found!")
        endif()
    else()
        set(OBFUSCATOR_SRC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/${OBFUSCATOR}/src)
        set(OBFUSCATOR_BIN_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/${OBFUSCATOR}/src)
    endif()
endif()
endif()

# Include chosen obfuscator
message(STATUS "Using obfuscator: ${OBFUSCATOR}")
add_subdirectory(${OBFUSCATOR_SRC_FOLDER} ${OBFUSCATOR_BIN_FOLDER})