cmake_minimum_required(VERSION 2.8)
project(ws)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/ ${CMAKE_MODULE_PATH} )

include(setup)

find_package(Sqlite3 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(cryptopp REQUIRED)
find_package(OpenSSL REQUIRED)


ADD_DEFINITIONS( -std=c++11 )
ADD_DEFINITIONS( -DASIO_STANDALONE )

SET (SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include/ws)

add_subdirectory(src)
add_subdirectory(test)

include(install)
