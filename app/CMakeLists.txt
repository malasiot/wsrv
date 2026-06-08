include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(web_app app.cpp )
target_link_libraries(web_app wsrv)

