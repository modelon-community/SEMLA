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
project(LicenseManager)

# Use dummy license manager if none is selected
if(NOT DEFINED LICENSE_MANAGER)
    set(LICENSE_MANAGER testingdummy)
endif()

set(LICENSE_MANAGER_SRC_FOLDER ${CMAKE_SOURCE_DIR}/license_managers/${LICENSE_MANAGER}/src)
set(LICENSE_MANAGER_BIN_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/license_managers/${LICENSE_MANAGER}/src)

message(STATUS "License Managers searched folders: ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/license_managers")

# Check that given license manager exists, else list available ones
if(NOT EXISTS ${LICENSE_MANAGER_SRC_FOLDER}/CMakeLists.txt)
    if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/${LICENSE_MANAGER}/src/CMakeLists.txt)
        file(GLOB children RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
        file(GLOB children2 RELATIVE ${CMAKE_SOURCE_DIR}/license_managers ${CMAKE_SOURCE_DIR}/license_managers/*)
        set(children "${children};${children2}")
        set(dirlist "")
        set(found 0)
        foreach(child ${children})
            if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${child}/src/CMakeLists.txt)
                set(dirlist "${dirlist} ${child}")
                set(found 1)
            elseif(EXISTS ${CMAKE_SOURCE_DIR}/license_managers/${child}/src/CMakeLists.txt)
                set(dirlist "${dirlist} ${child}")
                set(found 1)
            endif()
        endforeach()
        if(found)
            message(FATAL_ERROR "No such license manager: ${LICENSE_MANAGER}
            Searched folders: ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/license_managers
    Available license managers are: ${dirlist}")
        else()
            message(FATAL_ERROR "No license manager found!")
        endif()
    else()
        set(LICENSE_MANAGER_SRC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/${LICENSE_MANAGER}/src)
        set(LICENSE_MANAGER_BIN_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/${LICENSE_MANAGER}/src)
    endif()
endif()

# Include chosen manager
message(STATUS "Using license manager: ${LICENSE_MANAGER}")
add_subdirectory(${LICENSE_MANAGER_SRC_FOLDER} ${LICENSE_MANAGER_BIN_FOLDER})