# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QTR-Modding/CLibUtilsQTR
    REF bfe4e9980794ad8b16b308095c96ea7e29812c18
    SHA512 0d09a44306090860fa76db8aad52ee2ea6e394631d57b246184c3b1852df2aefb3fe4ce8904df2c774ab6c8601ab3f581090b706932e4e3fce5480a2154199d2
    HEAD_REF main
)

# Install codes
set(CLibUtilsQTR_SOURCE	${SOURCE_PATH}/include/CLibUtilsQTR)
file(INSTALL ${CLibUtilsQTR_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
