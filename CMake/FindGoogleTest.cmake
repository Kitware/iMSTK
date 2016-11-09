#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(GoogleTest_INCLUDE_DIR
  NAMES
    gtest/gtest.h
  PATH_SUFFIXES
    googletest/include
    )
mark_as_advanced(GoogleTest_INCLUDE_DIR)
list(APPEND GoogleTest_INCLUDE_DIRS ${GoogleTest_INCLUDE_DIR})

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(GoogleTest_LIBRARY
  NAMES
    libgtest
    gtest
  )
mark_as_advanced(GoogleTest_LIBRARY)
list(APPEND GoogleTest_LIBRARIES ${GoogleTest_LIBRARY})

find_library(GoogleTest_main_LIBRARY
  NAMES
    libgtest_main
    gtest_main
  )
mark_as_advanced(GoogleTest_main_LIBRARY)
list(APPEND GoogleTest_LIBRARIES ${GoogleTest_main_LIBRARY})

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GoogleTest
  REQUIRED_VARS
    GoogleTest_INCLUDE_DIRS
    GoogleTest_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(GoogleTest_FOUND AND NOT TARGET GoogleTest)
  add_library(GoogleTest INTERFACE IMPORTED)
  set_target_properties(GoogleTest PROPERTIES
    INTERFACE_LINK_LIBRARIES "${GoogleTest_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GoogleTest_INCLUDE_DIR}"
  )
endif()
