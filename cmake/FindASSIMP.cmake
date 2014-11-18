find_path(ASSIMP_INCLUDE_DIR
  assimp/config.h)
find_library(ASSIMP_LIBRARY
  NAMES
    assimp)

set(ASSIMP_INCLUDE_DIRS "${ASSIMP_INCLUDE_DIR}")
set(ASSIMP_LIBRARIES "${ASSIMP_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASSIMP
  REQUIRED_VARS
    ASSIMP_INCLUDE_DIR
    ASSIMP_LIBRARY)

mark_as_advanced(
  ASSIMP_INCLUDE_DIR
  ASSIMP_LIBRARY)