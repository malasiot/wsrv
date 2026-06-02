cmake_minimum_required(VERSION 3.10)
project(wsrv LANGUAGES CXX C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/ ${CMAKE_MODULE_PATH} )

include(setup)

find_package(Sqlite3 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CryptoPP REQUIRED)
find_package(OpenSSL REQUIRED)

INCLUDE_DIRECTORIES(twig/include ${SQLITE3_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${CryptoPP_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

add_definitions( -DASIO_STANDALONE )

SET (SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include/wsrv)

# Fetch and configure Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_subdirectory(twig)
add_subdirectory(src)
add_subdirectory(test)

include(install)
