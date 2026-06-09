include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(libs/xdb)

add_executable(web_app app.cpp routes.cpp )
target_link_libraries(web_app wsrv xdb)

