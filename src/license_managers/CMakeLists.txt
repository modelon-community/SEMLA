cmake_minimum_required(VERSION 2.8)
project(LicenseManager)

message(STATUS "License manager: ${LICENSE_MANAGER}")

if(NOT LICENSE_MANAGER)
    message(FATAL_ERROR "LICENSE_MANAGER not set")
elseif(EXISTS ${LICENSE_MANAGER}/src/CMakeLists.txt)
    add_subdirectory("${LICENSE_MANAGER}/src" "${CMAKE_CURRENT_BINARY_DIR}/license_manager")
else()
    message(FATAL_ERROR "Can't find CMakefile for LICENSE_MANAGER: ${LICENSE_MANAGER}")
endif()
