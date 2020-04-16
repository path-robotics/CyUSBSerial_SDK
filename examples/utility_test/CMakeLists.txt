add_executable(utility_test
    utility_test.cpp
    )

target_include_directories(utility_test
       PRIVATE
       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
       $<INSTALL_INTERFACE:include>
       )

target_link_libraries(utility_test
       PUBLIC
       ${PROJECT_NAME}::cyusb
       )
