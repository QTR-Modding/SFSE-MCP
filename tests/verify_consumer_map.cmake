if(NOT DEFINED MAP_FILE OR NOT EXISTS "${MAP_FILE}")
    message(FATAL_ERROR "Consumer link map was not generated: ${MAP_FILE}")
endif()

file(READ "${MAP_FILE}" _map)
foreach(
    _forbidden
    IN ITEMS
        "imgui.cpp.obj"
        "imgui_draw.cpp.obj"
        "imgui_tables.cpp.obj"
        "imgui_widgets.cpp.obj"
        "SFSE-MCP-ImGui.lib"
)
    string(FIND "${_map}" "${_forbidden}" _found)
    if(NOT _found EQUAL -1)
        message(FATAL_ERROR "Consumer unexpectedly contains an ImGui implementation object: ${_forbidden}")
    endif()
endforeach()

string(FIND "${_map}" "header_compile.obj" _consumer_object)
if(_consumer_object EQUAL -1)
    message(FATAL_ERROR "Consumer object was not found in its own link map.")
endif()