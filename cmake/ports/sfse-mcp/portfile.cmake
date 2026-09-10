vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://github.com/QTR-Modding/SFSE-MCP.git
    REF abeee13a366d2a288cb682037c6ec5d577bba810
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_TESTING=OFF
        -DSFSEMCP_BUNDLE_SIGNING_HEADERS=OFF
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
        "${SOURCE_PATH}/LICENSE"
        "${SOURCE_PATH}/THIRD_PARTY_NOTICES"
)
