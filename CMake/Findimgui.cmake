include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers for imgui
#-----------------------------------------------------------------------------
imstk_find_header(imgui imgui.h imgui)
imstk_find_header_package(imgui)
#message(STATUS "imgui includes : ${IMGUI_INCLUDE_DIRS}")

if(FIND_IMGUI_SOURCE)
  #-----------------------------------------------------------------------------
  # Find Source
  # Since imgui has no CMake, code is pulled into an iMSTK component directly
  #-----------------------------------------------------------------------------
  find_path(IMGUI_SOURCE_DIR
    NAMES
      imgui.cpp
    PATHS
      ${CMAKE_BINARY_DIR}/../External/imgui/src
    NO_DEFAULT_PATH
    )
  mark_as_advanced(IMGUI_SOURCE_DIR)
  #message(STATUS "imgui source : ${IMGUI_SOURCE_DIR}")

  #-----------------------------------------------------------------------------
  # Find package
  #-----------------------------------------------------------------------------
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(imgui
    REQUIRED_VARS
      IMGUI_SOURCE_DIR)
endif()
