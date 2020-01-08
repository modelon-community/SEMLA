cmake_minimum_required(VERSION 2.8)
project(Obfuscator)

if(NOT OBFUSCATOR)
    message(FATAL_ERROR "OBFUSCATOR not set. 
For default configure with: -DOBFUSCATOR=dummy
For custom configure with:  -DOBFUSCATOR=<path to obfuscator>")

elseif(EXISTS ${OBFUSCATOR}/src/CMakeLists.txt)
	add_subdirectory("${OBFUSCATOR}/src" "${CMAKE_CURRENT_BINARY_DIR}/obfuscator")

elseif(EXISTS ${CMAKE_SOURCE_DIR}/obfuscators/${OBFUSCATOR}/src/CMakeLists.txt)
	add_subdirectory("${CMAKE_SOURCE_DIR}/obfuscators/${OBFUSCATOR}/src" "${CMAKE_CURRENT_BINARY_DIR}/obfuscator")

else()	
	message(FATAL_ERROR "No such obfuscator: ${OBFUSCATOR}")

endif()

message(STATUS "Obfuscator: ${OBFUSCATOR}")
