SET ( SOURCES
    ${INCLUDE_ROOT}/detail/connection.hpp
    ${INCLUDE_ROOT}/detail/connection_manager.hpp
    ${INCLUDE_ROOT}/detail/io_service_pool.hpp
    ${INCLUDE_ROOT}/response.hpp
    ${INCLUDE_ROOT}/request_handler.hpp
    ${INCLUDE_ROOT}/request.hpp
    ${INCLUDE_ROOT}/detail/request_parser.hpp
    ${INCLUDE_ROOT}/server.hpp
    ${INCLUDE_ROOT}/detail/http_parser.h
    ${INCLUDE_ROOT}/session_handler.hpp
#    ${INCLUDE_ROOT}/fs_session_handler.hpp
    ${INCLUDE_ROOT}/session.hpp
    ${INCLUDE_ROOT}/route.hpp
    ${INCLUDE_ROOT}/router.hpp
    ${INCLUDE_ROOT}/filter.hpp
    ${INCLUDE_ROOT}/exceptions.hpp
#    ${INCLUDE_ROOT}/filters/request_logger.hpp
    ${INCLUDE_ROOT}/filters/static_file_handler.hpp
    ${INCLUDE_ROOT}/filters/gzip_filter.hpp

    ${SRC_ROOT}/connection_manager.cpp
    ${SRC_ROOT}/io_service_pool.cpp

    ${SRC_ROOT}/response.cpp
    ${SRC_ROOT}/request.cpp
    ${SRC_ROOT}/request_parser.cpp
    ${SRC_ROOT}/server.cpp
    ${SRC_ROOT}/http_parser.c
    ${SRC_ROOT}/session_handler.cpp
#    ${SRC_ROOT}/fs_session_handler.cpp
    ${SRC_ROOT}/session.cpp
    ${SRC_ROOT}/route.cpp
    ${SRC_ROOT}/filter_chain.cpp
    ${SRC_ROOT}/exceptions.cpp
    ${SRC_ROOT}/dictionary.cpp

#    ${SRC_ROOT}/filters/request_logger.cpp
    ${SRC_ROOT}/filters/static_file_handler.cpp
    ${SRC_ROOT}/filters/gzip_filter.cpp
)

ADD_LIBRARY(ws_http_server SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(ws_http_server ${Boost_LIBRARIES} dl z pthread )




