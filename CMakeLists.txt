cmake_minimum_required(VERSION 2.8)
project(wsrv)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/ ${CMAKE_MODULE_PATH} )

add_definitions(-std=c++11)

find_package(Libevent2 REQUIRED)

add_executable(main main.cpp workqueue.cpp workqueue.hpp threadpool.hpp)
target_link_libraries(main ${LIBEVENT2_LIBRARIES} -lpthread)
