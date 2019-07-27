#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)

file (GLOB_RECURSE GUI_h_files "${CMAKE_CURRENT_SOURCE_DIR}/imstk*.h")
file (GLOB_RECURSE GUI_cpp_files "${CMAKE_CURRENT_SOURCE_DIR}/imstk*.cpp")

if( iMSTK_USE_Vulkan )
  set (IMPLEMENTATION_H_FILES
    ${imgui_INCLUDE_DIR}/examples/imgui_impl_vulkan.h
    ${imgui_INCLUDE_DIR}/examples/imgui_impl_glfw.h)
  set (IMPLEMENTATION_CPP_FILES
    ${imgui_INCLUDE_DIR}/examples/imgui_impl_vulkan.cpp
    ${imgui_INCLUDE_DIR}/examples/imgui_impl_glfw.cpp)
endif()

imstk_add_library( GUIOverlay
  H_FILES
    ${imgui_INCLUDE_DIR}/imconfig.h
    ${imgui_INCLUDE_DIR}/imgui.h
    ${imgui_INCLUDE_DIR}/imgui_internal.h
    ${imgui_INCLUDE_DIR}/imstb_rectpack.h
    ${imgui_INCLUDE_DIR}/imstb_textedit.h
    ${imgui_INCLUDE_DIR}/imstb_truetype.h
    ${IMPLEMENTATION_H_FILES}
    ${GUI_h_files}
  CPP_FILES
    ${imgui_INCLUDE_DIR}/imgui.cpp
    ${imgui_INCLUDE_DIR}/imgui_demo.cpp
    ${imgui_INCLUDE_DIR}/imgui_draw.cpp
    ${imgui_INCLUDE_DIR}/imgui_widgets.cpp
    ${IMPLEMENTATION_CPP_FILES}
    ${GUI_cpp_files}
  DEPENDS
    Core
    imgui
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
