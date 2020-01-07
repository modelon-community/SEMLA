cmake_minimum_required(VERSION 2.8)
project(Decryptor)

if(NOT DECRYPTOR)
    message(FATAL_ERROR "DECRYPTOR not set. 
For default configure with: -DDECRYPTOR=default
For custom configure with:  -DDECRYPTOR=<path to decryptor>")

elseif(EXISTS ${DECRYPTOR}/src/CMakeLists.txt)
	add_subdirectory("${DECRYPTOR}/src" "${CMAKE_CURRENT_BINARY_DIR}/decryptor")

elseif(EXISTS ${CMAKE_SOURCE_DIR}/decryptors/${DECRYPTOR}/src/CMakeLists.txt)
	add_subdirectory("${CMAKE_SOURCE_DIR}/decryptors/${DECRYPTOR}/src" "${CMAKE_CURRENT_BINARY_DIR}/decryptor")

else()	
	message(FATAL_ERROR "No such decryptor: ${DECRYPTOR}")

endif()

message(STATUS "Decryptor: ${DECRYPTOR}")
