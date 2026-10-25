# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QTR-Modding/CLibUtilsQTR
    REF 69b25fea84dd23cef287d3ca20ccd3d348c09cb1
    SHA512 6c94072874b1f829ec424a0a910bd1d0128805e82581deb4690ee630ef179e91bda6aec6da664b08ae6073c92840c0104c6a7c98cb103043f66a283fe5cbb42a
    HEAD_REF main
)

# Install codes
set(CLibUtilsQTR_SOURCE	${SOURCE_PATH}/include/CLibUtilsQTR)
file(INSTALL ${CLibUtilsQTR_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
