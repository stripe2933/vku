vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Microsoft/vcpkg-docs
    REF "${VERSION}"
    SHA512 0  # This is a temporary value. We will modify this value in the next section.
    HEAD_REF vku
)


vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME "vku")

file(INSTALL "${SOURCE_PATH}/MIT-LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
configure_file("usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)