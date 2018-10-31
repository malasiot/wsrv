SET ( SOURCES
    detail/connection.hpp
    detail/connection_manager.hpp
    detail/io_service_pool.hpp
    detail/request_parser.hpp
    detail/http_parser.h
    detail/util.hpp

    ${INCLUDE_ROOT}/response.hpp
    ${INCLUDE_ROOT}/request_handler.hpp
    ${INCLUDE_ROOT}/request.hpp
    ${INCLUDE_ROOT}/server.hpp

    server_impl.hpp

    ${INCLUDE_ROOT}/session_handler.hpp
    ${INCLUDE_ROOT}/fs_session_handler.hpp
    ${INCLUDE_ROOT}/session.hpp
    ${INCLUDE_ROOT}/route.hpp
    ${INCLUDE_ROOT}/filter.hpp
    ${INCLUDE_ROOT}/exceptions.hpp
    ${INCLUDE_ROOT}/filters/request_logger.hpp
    ${INCLUDE_ROOT}/filters/static_file_handler.hpp
    ${INCLUDE_ROOT}/filters/gzip_filter.hpp

    connection_manager.cpp
    io_service_pool.cpp
    server.cpp
    filesystem.cpp
    crypto.cpp
    util.cpp

    response.cpp
    request.cpp
    request_parser.cpp
    server_impl.cpp
    http_parser.c
    session_handler.cpp
    fs_session_handler.cpp
    session.cpp
    route.cpp
    filter_chain.cpp
    exceptions.cpp
    dictionary.cpp

    filters/request_logger.cpp
    filters/static_file_handler.cpp
    filters/gzip_filter.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(ws_http_server SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(ws_http_server ${CRYPTOPP_LIBRARIES} ${SQLITE3_LIBRARY} z )




