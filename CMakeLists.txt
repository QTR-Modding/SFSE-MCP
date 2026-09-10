cmake_minimum_required(VERSION 3.25)

project(
    SFSE-MCP
    VERSION 0.12.0
    DESCRIPTION "Header-only client SDK for SFSE Menu Framework"
    LANGUAGES CXX
)

foreach(_sfsemcp_manifest IN ITEMS vcpkg.json cmake/ports/sfse-mcp/vcpkg.json)
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/${_sfsemcp_manifest}" _sfsemcp_manifest_json)
    string(JSON _sfsemcp_manifest_version GET "${_sfsemcp_manifest_json}" version-semver)
    if(NOT _sfsemcp_manifest_version VERSION_EQUAL PROJECT_VERSION)
        message(FATAL_ERROR
            "${_sfsemcp_manifest} version ${_sfsemcp_manifest_version} does not match ${PROJECT_VERSION}.")
    endif()
endforeach()

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported.")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(
    SFSEMCP_PUBLIC_HEADERS
    include/SFSEMCP/SFSEMenuFramework.hpp
    include/SFSEMCP/utils.hpp
)

add_library(SFSE-MCP INTERFACE ${SFSEMCP_PUBLIC_HEADERS})
add_library(SFSE-MCP::SFSE-MCP ALIAS SFSE-MCP)

find_path(SFSEMCP_SIGNING_INCLUDE_DIR CLibUtilsQTR/Signing.hpp
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/clib-utils-qtr/include" REQUIRED)
option(SFSEMCP_BUNDLE_SIGNING_HEADERS "Install the required QTR signing headers alongside the SDK" ON)

target_compile_features(SFSE-MCP INTERFACE cxx_std_23)
target_include_directories(
    SFSE-MCP
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${SFSEMCP_SIGNING_INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

get_target_property(_sfsemcp_target_type SFSE-MCP TYPE)
if(NOT _sfsemcp_target_type STREQUAL "INTERFACE_LIBRARY")
    message(FATAL_ERROR "SFSE-MCP must remain header-only.")
endif()
get_target_property(_sfsemcp_link_libraries SFSE-MCP INTERFACE_LINK_LIBRARIES)
if(_sfsemcp_link_libraries)
    message(FATAL_ERROR "SFSE-MCP must not transitively link an ImGui implementation or any other library.")
endif()

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

set(_sfsemcp_cmake_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/SFSE-MCP")

install(TARGETS SFSE-MCP EXPORT SFSE-MCPTargets)
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
if(SFSEMCP_BUNDLE_SIGNING_HEADERS)
    install(FILES "${SFSEMCP_SIGNING_INCLUDE_DIR}/CLibUtilsQTR/Signing.hpp"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/CLibUtilsQTR")
    install(DIRECTORY "${SFSEMCP_SIGNING_INCLUDE_DIR}/CLibUtilsQTR/Signing/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/CLibUtilsQTR/Signing"
        FILES_MATCHING PATTERN "*.hpp")
endif()
install(FILES LICENSE THIRD_PARTY_NOTICES DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/SFSE-MCP")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SFSE-MCPConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SFSE-MCPConfig.cmake"
    INSTALL_DESTINATION "${_sfsemcp_cmake_install_dir}"
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SFSE-MCPConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMinorVersion
)

install(
    EXPORT SFSE-MCPTargets
    FILE SFSE-MCPTargets.cmake
    NAMESPACE SFSE-MCP::
    DESTINATION "${_sfsemcp_cmake_install_dir}"
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/SFSE-MCPConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/SFSE-MCPConfigVersion.cmake"
    DESTINATION "${_sfsemcp_cmake_install_dir}"
)
