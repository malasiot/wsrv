cmake_minimum_required(VERSION 3.10)
project(wsrv LANGUAGES CXX C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/ ${CMAKE_MODULE_PATH} )

include(setup)

find_package(Sqlite3 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CryptoPP REQUIRED)
find_package(OpenSSL REQUIRED)

add_subdirectory(libs/twig)

set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JWT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)

add_subdirectory(3rdparty/jwt-cpp)

add_definitions( -DASIO_STANDALONE )

SET (SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include/wsrv)

INCLUDE_DIRECTORIES(include ${ZLIB_INCLUDE_DIR} ${CryptoPP_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})


# Fetch and configure Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()


add_subdirectory(src)
add_subdirectory(test)

include(install)
