include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(libs/xdb)

add_executable(web_app app.cpp routes.cpp util/xml_pull_parser.cpp util/gpx.cpp
    context.cpp
    util/string_util.cpp)
target_link_libraries(web_app wsrv xdb)

