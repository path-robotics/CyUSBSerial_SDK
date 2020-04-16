add_executable(controller_ex
    controller_ex.cpp
    )
target_include_directories(controller_ex
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

target_link_libraries(controller_ex
    PUBLIC
    ${PROJECT_NAME}::cyusb
    )
