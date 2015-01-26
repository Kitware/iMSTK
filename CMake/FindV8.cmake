find_path(V8_INCLUDE_DIR
  v8.h)
find_library(V8_LIBRARY
  NAMES
    v8
    v8_base)

set(V8_INCLUDE_DIRS "${V8_INCLUDE_DIR}")
set(V8_LIBRARIES "${V8_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(V8
  REQUIRED_VARS
    V8_INCLUDE_DIR
    V8_LIBRARY)

mark_as_advanced(
  V8_INCLUDE_DIR
  V8_LIBRARY)

if(V8_FOUND AND NOT TARGET v8::v8)
  add_library(v8::v8 INTERFACE IMPORTED)
  set_target_properties(v8::v8 PROPERTIES
    INTERFACE_LINK_LIBRARIES "${V8_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${V8_INCLUDE_DIR}")
endif()
