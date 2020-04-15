cmake_minimum_required(VERSION 3.14.0)
project(CyUSBSerial VERSION 1.0.0 LANGUAGES  C CXX)

set(${PROJECT_NAME}_DIR ${CMAKE_CURRENT_SOURCE_DIR})


set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "possible configurations" FORCE)

#### ---[ Find universal dependencies
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

# # ---[ Include pkgconfig
# include(FindPkgConfig)
#add_definitions(-D)
# ---[ Release/Debug specific flags
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_definitions("-DBOOST_DISABLE_ASSERTS -DEIGEN_NO_DEBUG ")
    #  add_compile_options(-std=c++17    -fopenmp) # -msse2 -O3 -mfpmath=sse )

endif()

IF( NOT CMAKE_BUILD_TYPE )
    SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type default to RelWithDebInfo, set to Release to improve performance" FORCE)
ENDIF()

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Maintainer."
        FORCE)



# Set the default c++ standard
# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


set(CMAKE_CXX_STANDARD 14)
# Building the libraries and executables
# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

# This variable will accumulate a list of components
# as the sub-directories are traversed
set(components)

#add_subdirectory(third_party)
add_subdirectory(modules/cyusb)
add_subdirectory(examples/utility_test)
if(NOT CMAKE_BUILD_TYPE MATCHES .*Rel.*)
    #add_subdirectory(apps)
    #add_subdirectory(examples)
endif()

message("${PROJECT_NAME} configured components:")
foreach(_comp ${components})
    message("  - ${_comp}")
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
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library for Cypress USB Serial Drivers.")
set(CPACK_PACKAGE_CONTACT "developers@path-robotics.com")
#set(CPACK_RESOURCE_FILE_LIECENSE "")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "/usr")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libusb-1.0-0-dev (>= 2:1.0.9)")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/deb/postinst;${CMAKE_CURRENT_SOURCE_DIR}/deb/postrm")

include(CPack)
