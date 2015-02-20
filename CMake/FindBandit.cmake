find_path(BANDIT_INCLUDE_DIR
	bandit/bandit.h)

set(BANDIT_INCLUDE_DIRS "${BANDIT_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Bandit
  REQUIRED_VARS
    BANDIT_INCLUDE_DIR)

mark_as_advanced(
  BANDIT_INCLUDE_DIR)
  