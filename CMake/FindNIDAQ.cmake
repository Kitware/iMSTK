find_path(NIDAQ_INCLUDE_DIR
  NIDAQmx.h)
find_library(NIDAQ_mx_LIBRARY
  NAMES
    NIDAQmx)
find_library(NIDAQ_syscfg_LIBRARY
  NAMES
    nisyscfg)

set(NIDAQ_INCLUDE_DIRS "${NIDAQ_INCLUDE_DIR}")
set(NIDAQ_LIBRARIES "${NIDAQ_mx_LIBRARY}" "${NIDQA_syscfg_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NIDAQ
  REQUIRED_VARS
    NIDAQ_INCLUDE_DIR
    NIDAQ_mx_LIBRARY
    NIDAQ_syscfg_LIBRARY)

mark_as_advanced(
  NIDAQ_INCLUDE_DIR
  NIDAQ_mx_LIBRARY
  NIDAQ_syscfg_LIBRARY)