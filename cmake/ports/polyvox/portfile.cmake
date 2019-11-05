include(vcpkg_common_functions)

# else Linux desktop
vcpkg_download_distfile(
    SOURCE_ARCHIVE
    URLS https://public.highfidelity.com/dependencies/polyvox-master-2015-7-15.zip
    SHA512 cc04cd43ae74b9c7bb065953540c0048053fcba6b52dc4218b3d9431fba178d65ad4f6c53cc1122ba61d0ab4061e99a7ebbb15db80011d607c5070ebebf8eddc
    FILENAME polyvox.zip
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE ${SOURCE_ARCHIVE}
)

vcpkg_configure_cmake(
  SOURCE_PATH ${SOURCE_PATH}
  PREFER_NINJA
  OPTIONS -DENABLE_EXAMPLES=OFF -DENABLE_BINDINGS=OFF 
)

vcpkg_install_cmake()

file(INSTALL ${SOURCE_PATH}/LICENSE.TXT DESTINATION ${CURRENT_PACKAGES_DIR}/share/polyvox RENAME copyright)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)

# if (APPLE)
#   set(INSTALL_NAME_LIBRARY_DIR ${INSTALL_DIR}/lib)
#   ExternalProject_Add_Step(
#     ${EXTERNAL_NAME}
#     change-install-name-debug
#     COMMENT "Calling install_name_tool on libraries to fix install name for dylib linking"
#     COMMAND ${CMAKE_COMMAND} -DINSTALL_NAME_LIBRARY_DIR=${INSTALL_NAME_LIBRARY_DIR}/Debug -P ${EXTERNAL_PROJECT_DIR}/OSXInstallNameChange.cmake
#     DEPENDEES install
#     WORKING_DIRECTORY <SOURCE_DIR>
#     LOG 1
#   )
#   ExternalProject_Add_Step(
#     ${EXTERNAL_NAME}
#     change-install-name-release
#     COMMENT "Calling install_name_tool on libraries to fix install name for dylib linking"
#     COMMAND ${CMAKE_COMMAND} -DINSTALL_NAME_LIBRARY_DIR=${INSTALL_NAME_LIBRARY_DIR}/Release -P ${EXTERNAL_PROJECT_DIR}/OSXInstallNameChange.cmake
#     DEPENDEES install
#     WORKING_DIRECTORY <SOURCE_DIR>
#     LOG 1
#   )
# endif ()
