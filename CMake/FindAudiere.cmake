find_path(AUDIERE_INCLUDE_DIR
  audiere.h)
find_library(AUDIERE_LIBRARY
  NAMES
    audiere)

set(AUDIERE_INCLUDE_DIRS "${AUDIERE_INCLUDE_DIR}")
set(AUDIERE_LIBRARIES "${AUDIERE_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Audiere
  REQUIRED_VARS
    AUDIERE_INCLUDE_DIR
    AUDIERE_LIBRARY)

mark_as_advanced(
  AUDIERE_INCLUDE_DIR
  AUDIERE_LIBRARY)

if(AUDIERE_FOUND AND NOT TARGET audiere::audiere)
  add_library(audiere::audiere INTERFACE IMPORTED)
  set_target_properties(audiere::audiere PROPERTIES
    INTERFACE_LINK_LIBRARIES "${AUDIERE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${AUDIERE_INCLUDE_DIR}")
endif()
