include_directories(
	${SQLITE3_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(test_url test_url.cpp )
target_link_libraries(test_url wsrv)


add_executable(test_server test_server.cpp )
target_link_libraries(test_server wsrv)

add_executable(test_file_upload test_file_upload.cpp )
target_link_libraries(test_file_upload wsrv)

add_executable(test_auth test_auth.cpp )
target_link_libraries(test_auth wsrv)


add_executable(test_mailer test_mailer.cpp )
target_link_libraries(test_mailer wsrv)

add_executable(test_client test_client.cpp )
target_link_libraries(test_client wsrv)

add_executable(test_forms test_forms.cpp )
target_link_libraries(test_forms wsrv)

add_executable(test_route test_route.cpp )
target_link_libraries(test_route wsrv gtest gtest_main)

add_test(NAME RouteTests COMMAND test_route)
target_link_libraries(test_client wsrv)
