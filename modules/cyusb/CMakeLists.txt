set(component cyusb)
set(components ${components} ${component} PARENT_SCOPE)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/" ${CMAKE_MODULE_PATH})

find_package(libusb ${MIN_LIBUSB_VERSION} REQUIRED)

add_library(${component} SHARED
        src/cyusb.cpp
        src/cyi2c.cpp
        src/cymisc.cpp
        src/cyjtag.cpp
        src/cyphdc.cpp
        src/cyspi.cpp
        src/cyuart.cpp
        src/CyController.cpp
        )

add_library(${PROJECT_NAME}::${component} ALIAS ${component})
target_compile_options(${component} PRIVATE -fPIC)
target_include_directories(${component}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_link_libraries(${component}
        PUBLIC
        libusb::libusb
        )

target_link_directories(${component}
        PUBLIC
        $<INSTALL_INTERFACE:lib/${PROJECT_NAME}>
        )

install(TARGETS ${component} EXPORT ${component}Targets
        COMPONENT ${component}
        LIBRARY DESTINATION lib/${PROJECT_NAME}
        ARCHIVE DESTINATION lib/${PROJECT_NAME}
        RUNTIME DESTINATION bin
        )

install(EXPORT ${component}Targets
        FILE "${PROJECT_NAME}-${component}Targets.cmake"
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
        COMPONENT ${component}
        )

configure_file("${component}Config.cmake.in"
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${component}Config.cmake"
        @ONLY
        )

install(
        FILES
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${component}Config.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
        COMPONENT ${component}
)

install(
        FILES
        "${CMAKE_CURRENT_LIST_DIR}/cmake/Findlibusb.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
        COMPONENT ${component}
)

install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        TYPE INCLUDE
        COMPONENT ${component}
        FILES_MATCHING PATTERN "*.hpp"
)

