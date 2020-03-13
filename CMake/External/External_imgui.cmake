#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_define_external_dirs( imgui )
# Gather up headers for install
set(imgui_H_FILES ${imgui_SOURCE_DIR}/imconfig.h
                  ${imgui_SOURCE_DIR}/imgui.h
                  ${imgui_SOURCE_DIR}/imstb_rectpack.h
                  ${imgui_SOURCE_DIR}/imstb_textedit.h
                  ${imgui_SOURCE_DIR}/imstb_truetype.h
   )
set(make_imgui_dir_command
  ${CMAKE_COMMAND} -E make_directory
  ${CMAKE_INSTALL_PREFIX}/include/imgui/
  )
set(copy_imgui_headers_command
  ${CMAKE_COMMAND} -E copy
  ${imgui_H_FILES}
  ${CMAKE_INSTALL_PREFIX}/include/imgui/
  )

imstk_add_external_project( imgui
  URL https://gitlab.kitware.com/iMSTK/imgui/-/archive/v1.65/imgui-v1.65.zip
  URL_MD5 e255a471fa1cc4f2be931659c253956a
  CONFIGURE_COMMAND ${SKIP_STEP_COMMAND}
  BUILD_COMMAND ${SKIP_STEP_COMMAND}
  INSTALL_COMMAND
    COMMAND ${make_imgui_dir_command}
    COMMAND ${copy_imgui_headers_command}
  RELATIVE_INCLUDE_PATH ""
  DEPENDENCIES ""
  #VERBOSE
  )
