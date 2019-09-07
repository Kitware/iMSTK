#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)

file (GLOB_RECURSE GUI_h_files "${CMAKE_CURRENT_SOURCE_DIR}/imstk*.h")
file (GLOB_RECURSE GUI_cpp_files "${CMAKE_CURRENT_SOURCE_DIR}/imstk*.cpp")

if( iMSTK_USE_Vulkan )
  set (IMPLEMENTATION_H_FILES
    ${IMGUI_SOURCE_DIR}/examples/imgui_impl_vulkan.h
    ${IMGUI_SOURCE_DIR}/examples/imgui_impl_glfw.h)
  set (IMPLEMENTATION_CPP_FILES
    ${IMGUI_SOURCE_DIR}/examples/imgui_impl_vulkan.cpp
    ${IMGUI_SOURCE_DIR}/examples/imgui_impl_glfw.cpp)
endif()

imstk_add_library( GUIOverlay
  H_FILES
    ${IMGUI_SOURCE_DIR}/imconfig.h
    ${IMGUI_SOURCE_DIR}/imgui.h
    ${IMGUI_SOURCE_DIR}/imgui_internal.h
    ${IMGUI_SOURCE_DIR}/imstb_rectpack.h
    ${IMGUI_SOURCE_DIR}/imstb_textedit.h
    ${IMGUI_SOURCE_DIR}/imstb_truetype.h
    ${IMPLEMENTATION_H_FILES}
    ${GUI_h_files}
  CPP_FILES
    ${IMGUI_SOURCE_DIR}/imgui.cpp
    ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
    ${IMPLEMENTATION_CPP_FILES}
    ${GUI_cpp_files}
  DEPENDS
    Core
    imgui
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
