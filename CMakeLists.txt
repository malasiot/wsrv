cmake_minimum_required(VERSION 3.10)
project(wsrv LANGUAGES CXX C)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/ ${CMAKE_MODULE_PATH} )

include(setup)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH 
        "/opt/homebrew/opt/sqlite"     # Apple Silicon (M1/M2/M3/M4)
        "/usr/local/opt/sqlite"         # Intel Macs
    )
endif()

find_package(SQLite3 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CryptoPP REQUIRED)
find_package(OpenSSL REQUIRED)

if(NOT TARGET SQLite3::SQLite3 AND TARGET SQLite::SQLite3)
    add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
endif()

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
add_subdirectory(app)

include(install)
