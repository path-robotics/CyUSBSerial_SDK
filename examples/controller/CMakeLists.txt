
# find_package(yaml-cpp 0.5.2 REQUIRED)

# Online Calibration Routine Example
####################################################
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
add_executable(controller_ex
    controller_ex.cpp
    )
set(CONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(controller_ex PUBLIC "CONFIG_PATH=\"${CONFIG_PATH}\"")
#target_compile_options(welding_sim PUBLIC -std=c++14    -fopenmp)
target_include_directories(controller_ex
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

target_link_libraries(controller_ex
    PUBLIC
    ${PROJECT_NAME}::cyusb
    )
