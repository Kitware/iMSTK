find_path(VEGA_INCLUDE_DIR
  forceModel/forceModel.h)
find_library(VEGA_LIBRARY
  NAMES
    vega)

set(VEGA_INCLUDE_DIRS "${VEGA_INCLUDE_DIR}")
set(VEGA_LIBRARIES "${VEGA_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vega
  REQUIRED_VARS
    VEGA_INCLUDE_DIR
    VEGA_LIBRARY)

mark_as_advanced(
  VEGA_INCLUDE_DIR
  VEGA_LIBRARY)