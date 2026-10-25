# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QTR-Modding/CLibUtilsQTR
    REF 3af01ff24c133dce53f518677ec833b0778e8c75
    SHA512 b14ee76e70f66004df50e4e51d5b54aa57c4d15ddb07785a7a0c8be85beb3721d12033452e3d93fabd29fae0af137b6abbe37a7fed9e42e59b3d9198cf8df861
    HEAD_REF main
)

# Install codes
set(CLibUtilsQTR_SOURCE	${SOURCE_PATH}/include/CLibUtilsQTR)
file(INSTALL ${CLibUtilsQTR_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
