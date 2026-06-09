message(${INCLUDE_DIR})

set( SOURCES
    connection.cpp
    driver_factory.cpp
    exception.cpp
    statement.cpp
    transaction.cpp
    query.cpp
    
    query_result.cpp
    statement_handle.cpp

    drivers/sqlite/driver.cpp
    drivers/sqlite/connection.cpp
    drivers/sqlite/exceptions.cpp
    drivers/sqlite/statement.cpp
    drivers/sqlite/query_result.cpp
    drivers/sqlite/statement.hpp

    ${INCLUDE_DIR}/query.hpp
    ${INCLUDE_DIR}/connection.hpp
    ${INCLUDE_DIR}/exception.hpp
    ${INCLUDE_DIR}/statement.hpp
    ${INCLUDE_DIR}/transaction.hpp
    ${INCLUDE_DIR}/query_result.hpp
    ${INCLUDE_DIR}/statement_handle.hpp

)

#if ( PostgreSQL_FOUND )
#    add_definitions("-DHAS_PGSQL_DRIVER")
#    list(APPEND SOURCES
#        drivers/pgsql/driver.cpp
#        drivers/pgsql/connection.cpp
#        drivers/pgsql/exceptions.cpp
#        drivers/pgsql/statement.cpp
#        drivers/pgsql/query_result.cpp
#        drivers/pgsql/parameters.cpp
#)
#endif ( PostgreSQL_FOUND )

if ( TARGET PkgConfig::MARIADB )
    list(APPEND SOURCES
        drivers/mysql/driver.cpp
        drivers/mysql/connection.cpp
        drivers/mysql/statement.cpp
        drivers/mysql/query_result.cpp
    )
endif()

add_library(xdb SHARED ${SOURCES})

include_directories(
   ${SQLite3_INCLUDE_DIRS}
   ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(xdb
    PRIVATE SQLite::SQLite3 dl z pthread
)

target_include_directories(xdb PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../include 
  )

#if ( PostgreSQL_FOUND )
#    target_link_libraries(xdb PRIVATE PostgreSQL::PostgreSQL)
#    target_compile_definitions(xdb PRIVATE HAS_PGSQL_DRIVER)
#endif (  PostgreSQL_FOUND )  

if ( TARGET PkgConfig::MARIADB )
    target_link_libraries(xdb PRIVATE PkgConfig::MARIADB)
    target_compile_definitions(xdb PRIVATE HAS_MYSQL_DRIVER)
endif (  )  

# Install library
install(TARGETS xdb
  EXPORT ${PROJECT_NAME}-export
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT stlib
  COMPONENT dev)

# Install headers
install(DIRECTORY "${LIBRARY_INCLUDE_DIR}" DESTINATION "${INSTALL_INCLUDE_DIR}/xdb" )


