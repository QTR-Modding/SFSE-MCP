add_executable(sfsemcp_header_compile header_compile.cpp)
target_link_libraries(sfsemcp_header_compile PRIVATE SFSE-MCP::SFSE-MCP)

add_library(sfsemcp_mock_host SHARED mock_host.cpp)
set_target_properties(sfsemcp_mock_host PROPERTIES OUTPUT_NAME SFSEMenuFramework)

add_executable(sfsemcp_wrapper_mock wrapper_mock.cpp)
target_link_libraries(sfsemcp_wrapper_mock PRIVATE SFSE-MCP::SFSE-MCP)
add_dependencies(sfsemcp_wrapper_mock sfsemcp_mock_host)

add_library(sfsemcp_legacy_mock_host SHARED legacy_mock_host.cpp)
set_target_properties(
    sfsemcp_legacy_mock_host
    PROPERTIES
        OUTPUT_NAME SFSEMenuFramework
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/legacy"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/legacy"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/legacy"
)

add_executable(sfsemcp_wrapper_legacy wrapper_legacy.cpp)
target_link_libraries(sfsemcp_wrapper_legacy PRIVATE SFSE-MCP::SFSE-MCP)
add_dependencies(sfsemcp_wrapper_legacy sfsemcp_legacy_mock_host)

foreach(_sfsemcp_test_target IN ITEMS
        sfsemcp_header_compile
        sfsemcp_mock_host
        sfsemcp_wrapper_mock
        sfsemcp_legacy_mock_host
        sfsemcp_wrapper_legacy)
    if(MSVC)
        target_compile_options(${_sfsemcp_test_target} PRIVATE /W4 /WX /permissive- /EHsc)
    endif()
endforeach()

if(MSVC)
    target_link_options(
        sfsemcp_header_compile
        PRIVATE
            "/MAP:$<TARGET_FILE_DIR:sfsemcp_header_compile>/sfsemcp_consumer.map"
    )
endif()

add_test(NAME sfsemcp.header_compile COMMAND sfsemcp_header_compile)
add_test(
    NAME sfsemcp.wrapper_dispatch
    COMMAND sfsemcp_wrapper_mock "$<TARGET_FILE:sfsemcp_mock_host>"
)
add_test(
    NAME sfsemcp.legacy_host_fallback
    COMMAND sfsemcp_wrapper_legacy "$<TARGET_FILE:sfsemcp_legacy_mock_host>"
)

if(MSVC)
    add_test(
        NAME sfsemcp.no_imgui_implementation
        COMMAND
            "${CMAKE_COMMAND}"
            "-DMAP_FILE=$<TARGET_FILE_DIR:sfsemcp_header_compile>/sfsemcp_consumer.map"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/verify_consumer_map.cmake"
    )
endif()
