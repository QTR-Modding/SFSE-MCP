if(NOT DEFINED MAP_FILE OR NOT EXISTS "${MAP_FILE}")
    message(FATAL_ERROR "Consumer link map was not generated: ${MAP_FILE}")
endif()

file(READ "${MAP_FILE}" _map)
string(REGEX MATCH [[imgui(_draw|_tables|_widgets)?(\.cpp)?\.obj|SFSE-MCP-ImGui\.lib]] _implementation "${_map}")
if(_implementation)
    message(FATAL_ERROR "Consumer unexpectedly contains an ImGui implementation object: ${_implementation}")
endif()

if(NOT _map MATCHES [[header_compile(\.cpp)?\.obj]])
    message(FATAL_ERROR "Consumer object was not found in its own link map.")
endif()
