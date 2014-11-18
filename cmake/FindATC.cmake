find_path(ATC_INCLUDE_DIR
  ATC3DG.h)
find_library(ATC_LIBRARY
  NAMES
    ATC3DG)

set(ATC_INCLUDE_DIRS "${ATC_INCLUDE_DIR}")
set(ATC_LIBRARIES "${ATC_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ATC
  REQUIRED_VARS
    ATC_INCLUDE_DIR
    ATC_LIBRARY)

mark_as_advanced(
  ATC_INCLUDE_DIR
  ATC_LIBRARY)