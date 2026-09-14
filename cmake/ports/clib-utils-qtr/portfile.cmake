# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QTR-Modding/CLibUtilsQTR
    REF 615f85c09df194fd5e86bf643a6a275521217e5a
    SHA512 97e0be2455a48d20bc776251c27152760f4bf5c5e2b61bfe1739a4fc3aec5df950dc39145eda837bdad5b2d55f5570c565be892df1bcf28dd848df01da6d5fbe
    HEAD_REF main
)

# Install codes
set(CLibUtilsQTR_SOURCE	${SOURCE_PATH}/include/CLibUtilsQTR)
file(INSTALL ${CLibUtilsQTR_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
