cmake_minimum_required(VERSION 3.14.0)
project(CyUSBSerial VERSION 1.0.5 LANGUAGES C CXX)

set(${PROJECT_NAME}_DIR ${CMAKE_CURRENT_SOURCE_DIR})
option(BUILD_EXAMPLES "Build the examples" OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 14)

set(components)

add_subdirectory(modules/cyusb)

if(BUILD_EXAMPLES)
  add_subdirectory(examples/utility_test)
  add_subdirectory(examples/controller)
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES .*Rel.*)

endif()

message(STATUS "${PROJECT_NAME} configured components:")
foreach(_comp ${components})
  message(STATUS "  ${_comp}")
endforeach()

configure_file(${PROJECT_NAME}Config.cmake.in
 "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
 @ONLY
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION lib/cmake/${PROJECT_NAME}
)

# CPack Options for generating installers
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VENDOR "Path Robotics")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/deb/doc")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library for Cypress USB Serial Drivers.")
set(CPACK_PACKAGE_CONTACT "dylan@path-robotics.com")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "/usr")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libusb-1.0-0-dev (>= 2:1.0.9), udev")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst;${CMAKE_CURRENT_SOURCE_DIR}/deb/postrm")

include(CPack)
