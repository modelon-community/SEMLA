cmake_minimum_required(VERSION 2.8)
project(LicenseManager)

if(NOT LICENSE_MANAGER)
    message(FATAL_ERROR "LICENSE_MANAGER not set. 
For default configure with: -DLICENSE_MANAGER=testingdummy
For custom configure with:  -DLICENSE_MANAGER=<path to license manager>")

elseif(EXISTS ${LICENSE_MANAGER}/src/CMakeLists.txt)
    add_subdirectory("${LICENSE_MANAGER}/src""${CMAKE_CURRENT_BINARY_DIR}/license_manager")
	
elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${LICENSE_MANAGER}/src/CMakeLists.txt)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${LICENSE_MANAGER}/src" "${CMAKE_CURRENT_BINARY_DIR}/license_manager")
	
else()
    message(FATAL_ERROR "No such license manager: ${LICENSE_MANAGER}")
	
endif()

message(STATUS "License manager: ${LICENSE_MANAGER}")
