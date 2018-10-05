#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(imgui_INCLUDE_DIR
  NAMES
    imgui.h
    )
mark_as_advanced(imgui_INCLUDE_DIR)
message(STATUS "imgui_INCLUDE_DIR : ${imgui_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(imgui
  REQUIRED_VARS
    imgui_INCLUDE_DIR)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(GLM_FOUND AND NOT TARGET imgui)
  add_library(imgui INTERFACE IMPORTED)
endif()
