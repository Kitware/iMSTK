include(FetchContent)


# Download project template
set(proj "iMSTKExternalProjectTemplate")
set(templates_src_dir "${CMAKE_CURRENT_BINARY_DIR}/${proj}")
FetchContent_Populate(${proj}
  SOURCE_DIR     "${templates_src_dir}"
  GIT_REPOSITORY https://gitlab.kitware.com/iMSTK/imstkexternalprojecttemplate
  GIT_TAG        8a90e0b67da782ee52f61d7e982ee64ade7f1c17
  QUIET
  )

imstk_add_project_test(iMSTKBuildSystemTest.iMSTKProjectAgainstBuildTree
  SOURCE_DIR ${templates_src_dir}/iMSTKProject
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/iMSTKProjectAgainstBuildTree
  BUILD_OPTIONS
    -DiMSTK_DIR:PATH=${iMSTK_BINARY_DIR}
  WORKING_DIR ${CMAKE_INSTALL_PREFIX}/bin
  )

imstk_add_project_test(iMSTKBuildSystemTest.iMSTKProjectAgainstInstallTree
  SOURCE_DIR ${templates_src_dir}/iMSTKProject
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/iMSTKProjectAgainstInstallTree
  BUILD_OPTIONS
    -DiMSTK_DIR:PATH=${CMAKE_INSTALL_PREFIX}/lib/cmake/iMSTK-${iMSTK_VERSION_MAJOR}.${iMSTK_VERSION_MINOR}
  WORKING_DIR ${CMAKE_INSTALL_PREFIX}/bin
  )
