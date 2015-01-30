find_path(ADU_INCLUDE_DIR
  AduHid.h)
find_library(ADU_LIBRARY
  NAMES
    aduhid)

set(ADU_INCLUDE_DIRS "${ADU_INCLUDE_DIR}")
set(ADU_LIBRARIES "${ADU_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ADU
  REQUIRED_VARS
    ADU_INCLUDE_DIR
    ADU_LIBRARY)

mark_as_advanced(
  ADU_INCLUDE_DIR
  ADU_LIBRARY)