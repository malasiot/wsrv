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

    ${INCLUDE_ROOT}/session_manager.hpp
    ${INCLUDE_ROOT}/sqlite3_session_manager.hpp
    ${INCLUDE_ROOT}/session.hpp
    ${INCLUDE_ROOT}/route.hpp

    ${INCLUDE_ROOT}/exceptions.hpp

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
    session_manager.cpp
    sqlite3_session_manager.cpp
    session.cpp
    route.cpp

    exceptions.cpp
    dictionary.cpp
)

ADD_LIBRARY(ws_http_server ${LIBRARY_TYPE} ${SOURCES})

target_include_directories(ws_http_server PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  ${CMAKE_SOURCE_DIR}/src
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

target_include_directories(ws_http_server PUBLIC
    ${SQLITE3_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)


target_link_libraries(ws_http_server PUBLIC
      ${CRYPTOPP_LIBRARIES} ${SQLITE3_LIBRARY} z
)

# Install binaries
install(TARGETS ws_http_server
  EXPORT ${PROJECT_NAME}-export
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  COMPONENT dev)

# Install headers
install(DIRECTORY "${LIBRARY_INCLUDE_DIR}" DESTINATION "${INSTALL_INCLUDE_DIR}/ws" )


