set(component cyusb-tools)
set(components ${components} ${component} PARENT_SCOPE)

function(add_tool toolname)
    add_executable(${toolname}
            src/${toolname}.cpp
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
add_tool(lscygpio)
