set(component cyusb-tools)
set(components ${components} ${component} PARENT_SCOPE)

function(add_tool toolname)
    add_executable(${toolname}
            src/${toolname}.cpp
            )
    target_include_directories(${toolname}
            PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
            )
    target_link_libraries(${toolname}
            PUBLIC
            cyusb
            )
    install(TARGETS ${toolname}
            COMPONENT ${component}
            RUNTIME DESTINATION bin
            )
endfunction()

add_tool(cygpio-hammer)
add_tool(cygpioget)
add_tool(cygpioset)
add_tool(lscygpio)
