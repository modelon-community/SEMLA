if (USE_CUSTOM_OPENSSL_SUBDIRECTORY)
    # do nothing
elseif (USE_DOWNLOADED_OPENSSL_BUILD)
    if(NOT DOWNLOADED_OPENSSL_BUILD_URL)
        message(FATAL_ERROR "DOWNLOADED_OPENSSL_BUILD_URL not set")
    endif()
    if(NOT DOWNLOADED_OPENSSL_BUILD_EXPECTED_HASH_SHA256)
        message(FATAL_ERROR "DOWNLOADED_OPENSSL_BUILD_EXPECTED_HASH_SHA256 not set")
    endif()

    string(REGEX REPLACE ".*/" "" DOWNLOADED_OPENSSL_BUILD_FILENAME ${DOWNLOADED_OPENSSL_BUILD_URL})

    file(DOWNLOAD
        ${DOWNLOADED_OPENSSL_BUILD_URL}
        ${DOWNLOADED_OPENSSL_BUILD_FILENAME}
        EXPECTED_HASH SHA256=${DOWNLOADED_OPENSSL_BUILD_EXPECTED_HASH_SHA256}
        TLS_VERIFY ON
        NETRC OPTIONAL
    )
    file(ARCHIVE_EXTRACT INPUT ${DOWNLOADED_OPENSSL_BUILD_FILENAME}
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/openssl
    )
    add_library(ssl STATIC IMPORTED)
    add_library(crypto STATIC IMPORTED)
    file(GLOB LIBSSL_IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/openssl/lib/libssl*${CMAKE_STATIC_LIBRARY_SUFFIX})
    file(GLOB LIBCRYPTO_IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/openssl/lib/libcrypto*${CMAKE_STATIC_LIBRARY_SUFFIX})
    set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${LIBSSL_IMPORTED_LOCATION})
    set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${LIBCRYPTO_IMPORTED_LOCATION})
    set(DOWNLOADED_OPENSSL_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl/include)
    include_directories(${DOWNLOADED_OPENSSL_BUILD_INCLUDE_DIR})
    message(STATUS "Will use pre-built OpenSSL from ${DOWNLOADED_OPENSSL_BUILD_URL}")
else()
    include(BuildSemlaOpenSSL)
endif()
