find_path(GLM_INCLUDE_DIR
	glm.hpp
  PATH_SUFFIXES
	glm)

set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM
  REQUIRED_VARS
    GLM_INCLUDE_DIR)

mark_as_advanced(
  GLM_INCLUDE_DIR)
  