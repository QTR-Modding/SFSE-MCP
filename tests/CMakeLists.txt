set(SFSEMCP_TEST_SIGNING_KEY_HEADER "" CACHE FILEPATH "Build-local public key for signed test fixtures")
if(NOT EXISTS "${SFSEMCP_TEST_SIGNING_KEY_HEADER}")
    message(FATAL_ERROR "Run tests/run_signed_tests.ps1 to build and test with an isolated signing key.")
endif()
file(TO_CMAKE_PATH "${SFSEMCP_TEST_SIGNING_KEY_HEADER}" SFSEMCP_TEST_SIGNING_KEY_HEADER)
add_subdirectory("../tools" "${CMAKE_CURRENT_BINARY_DIR}/tools")

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

foreach(_sfsemcp_fixture IN ITEMS unsigned wrong_key)
    add_library(sfsemcp_${_sfsemcp_fixture}_mock_host SHARED mock_host.cpp)
    set_target_properties(sfsemcp_${_sfsemcp_fixture}_mock_host PROPERTIES
        OUTPUT_NAME SFSEMenuFramework
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_sfsemcp_fixture}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_sfsemcp_fixture}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_sfsemcp_fixture}")
endforeach()

add_executable(sfsemcp_provider_binding provider_binding.cpp)
target_link_libraries(sfsemcp_provider_binding PRIVATE SFSE-MCP::SFSE-MCP)
add_dependencies(sfsemcp_provider_binding sfsemcp_mock_host sfsemcp_unsigned_mock_host sfsemcp_wrong_key_mock_host)

foreach(_sfsemcp_test_target IN ITEMS
        sfsemcp_header_compile
        sfsemcp_mock_host
        sfsemcp_wrapper_mock
        sfsemcp_legacy_mock_host
        sfsemcp_wrapper_legacy
        sfsemcp_unsigned_mock_host
        sfsemcp_wrong_key_mock_host
        sfsemcp_provider_binding)
    target_compile_definitions(${_sfsemcp_test_target} PRIVATE
        "SFSEMCP_SIGNING_KEY_HEADER=\"${SFSEMCP_TEST_SIGNING_KEY_HEADER}\"")
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

foreach(_sfsemcp_mode IN ITEMS valid retry alias concurrent code exports teardown)
    add_test(NAME sfsemcp.provider_${_sfsemcp_mode}
        COMMAND sfsemcp_provider_binding "${_sfsemcp_mode}" "$<TARGET_FILE:sfsemcp_mock_host>")
    set_tests_properties(sfsemcp.provider_${_sfsemcp_mode} PROPERTIES TIMEOUT 30)
endforeach()
add_test(NAME sfsemcp.unsigned_provider_rejected
    COMMAND sfsemcp_provider_binding signature "$<TARGET_FILE:sfsemcp_unsigned_mock_host>")
set_tests_properties(sfsemcp.unsigned_provider_rejected PROPERTIES TIMEOUT 30)

set(_sfsemcp_tampered "${CMAKE_CURRENT_BINARY_DIR}/tampered/$<CONFIG>/SFSEMenuFramework.dll")
add_test(NAME sfsemcp.wrong_signer_rejected
    COMMAND sfsemcp_provider_binding signature "$<TARGET_FILE:sfsemcp_wrong_key_mock_host>")
add_test(NAME sfsemcp.modified_signed_file_rejected
    COMMAND sfsemcp_provider_binding signature "${_sfsemcp_tampered}")
add_test(NAME sfsemcp.genuine_file_bait_rejected
    COMMAND sfsemcp_provider_binding bait "${_sfsemcp_tampered}" "$<TARGET_FILE:sfsemcp_mock_host>")
set_tests_properties(sfsemcp.wrong_signer_rejected sfsemcp.modified_signed_file_rejected
    sfsemcp.genuine_file_bait_rejected PROPERTIES TIMEOUT 30)

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/signed-tests-$<CONFIG>.json" CONTENT
"{
  \"modern\": \"$<TARGET_FILE:sfsemcp_mock_host>\",
  \"legacy\": \"$<TARGET_FILE:sfsemcp_legacy_mock_host>\",
  \"unsigned\": \"$<TARGET_FILE:sfsemcp_unsigned_mock_host>\",
  \"wrong\": \"$<TARGET_FILE:sfsemcp_wrong_key_mock_host>\",
  \"tampered\": \"${_sfsemcp_tampered}\",
  \"verifier\": \"$<TARGET_FILE:sfsemcp_verify_signature>\"
}
")
