# This "exports" all targets which have been put into the export set

# Path of the CMake files generated
set(PROJECT_CMAKE_FILES ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY})

install(EXPORT ${PROJECT_NAME}-export
  DESTINATION ${INSTALL_CMAKE_DIR}
  FILE ${PROJECT_NAME}Targets.cmake)

message(${LIBRARY_NAME})
# Create the <package>Config.cmake.in
configure_file(${CMAKE_SOURCE_DIR}/CMake/Config.cmake.in
  "${PROJECT_CMAKE_FILES}/${PROJECT_NAME}Config.cmake" @ONLY)

# Install <package>Config.cmake and <package>ConfigVersion.cmake files
install(FILES
  "${PROJECT_CMAKE_FILES}/${PROJECT_NAME}Config.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# Uninstall targets
configure_file("${CMAKE_SOURCE_DIR}/CMake/Uninstall.cmake.in"
  "${PROJECT_CMAKE_FILES}/Uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${PROJECT_CMAKE_FILES}/Uninstall.cmake)
