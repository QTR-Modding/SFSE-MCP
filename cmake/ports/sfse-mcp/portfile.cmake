get_filename_component(SFSEMCP_ROOT "${CURRENT_PORT_DIR}/../../.." ABSOLUTE)

vcpkg_cmake_configure(
    SOURCE_PATH "${SFSEMCP_ROOT}"
    OPTIONS
        -DBUILD_TESTING=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(
    PACKAGE_NAME SFSE-MCP
    CONFIG_PATH lib/cmake/SFSE-MCP
)

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug"
    "${CURRENT_PACKAGES_DIR}/lib"
)
vcpkg_install_copyright(
    FILE_LIST
        "${SFSEMCP_ROOT}/LICENSE"
        "${SFSEMCP_ROOT}/THIRD_PARTY_NOTICES"
)
