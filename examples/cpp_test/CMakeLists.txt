
# find_package(yaml-cpp 0.5.2 REQUIRED)

# Online Calibration Routine Example
####################################################
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
add_executable(utility_test
    utility_test_two.cpp
    )
set(CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(utility_test PUBLIC "CONFIG_PATH=\"${CONFIG_PATH}\"")
#target_compile_options(welding_sim PUBLIC -std=c++14    -fopenmp)
target_include_directories(utility_test
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

target_link_libraries(utility_test
    PUBLIC
    ${PROJECT_NAME}::cyusb
    )
