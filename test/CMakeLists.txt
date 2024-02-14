include_directories(
	${SQLITE3_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions( -std=c++11 )

add_executable(test_url test_url.cpp )
target_link_libraries(test_url wsrv)

add_executable(test_server test_server.cpp )
target_link_libraries(test_server wsrv)

add_executable(test_mailer test_mailer.cpp )
target_link_libraries(test_mailer wsrv)

add_executable(test_client test_client.cpp )
target_link_libraries(test_client wsrv)
