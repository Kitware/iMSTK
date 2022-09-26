include(FetchContent)


# Download project template
set(proj "iMSTKExternalProjectTemplate")
set(templates_src_dir "${CMAKE_CURRENT_BINARY_DIR}/${proj}")
set(SHA e525e661300b978d2cdede812adfdf1c81ba8df8)
FetchContent_Populate(${proj}
  SOURCE_DIR     "${templates_src_dir}"
  URL https://gitlab.kitware.com/iMSTK/imstkexternalprojecttemplate/-/archive/${SHA}/imstkexternalprojecttemplate-${SHA}.zip
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
