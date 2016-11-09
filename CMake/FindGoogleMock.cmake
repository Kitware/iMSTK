#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(GoogleMock_INCLUDE_DIR
  NAMES
    gmock/gmock.h
  PATH_SUFFIXES
    googlemock/include
    )
mark_as_advanced(GoogleMock_INCLUDE_DIR)
list(APPEND GoogleMock_INCLUDE_DIRS ${GoogleMock_INCLUDE_DIR})

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(GoogleMock_LIBRARY
  NAMES
    libgmock
    gmock
  )
mark_as_advanced(GoogleMock_LIBRARY)
list(APPEND GoogleMock_LIBRARIES ${GoogleMock_LIBRARY})

find_library(GoogleMock_main_LIBRARY
  NAMES
    libgmock_main
    gmock_main
  )
mark_as_advanced(GoogleMock_main_LIBRARY)
list(APPEND GoogleMock_LIBRARIES ${GoogleMock_main_LIBRARY})

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GoogleMock
  REQUIRED_VARS
    GoogleMock_INCLUDE_DIRS
    GoogleMock_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(GoogleMock_FOUND AND NOT TARGET GoogleMock)
  add_library(GoogleMock INTERFACE IMPORTED)
  set_target_properties(GoogleMock PROPERTIES
    INTERFACE_LINK_LIBRARIES "${GoogleMock_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GoogleMock_INCLUDE_DIR}"
  )
endif()
