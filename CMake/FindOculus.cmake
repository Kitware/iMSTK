find_path(Oculus_INCLUDE_DIR
  NAMES
    OVR.h
    )

find_path(Oculus_SRC_DIR
  NAMES
    OVR_CAPI_GL.h
    )

find_library(Oculus_LIBRARY
  NAMES
    ovr
    ovrd)

set(Oculus_INCLUDE_DIRS "${Oculus_INCLUDE_DIR}" "${Oculus_SRC_DIR}")
set(Oculus_LIBRARIES "${Oculus_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Oculus
  REQUIRED_VARS
    Oculus_INCLUDE_DIR
    Oculus_LIBRARY)

mark_as_advanced(
  Oculus_INCLUDE_DIR
  Oculus_LIBRARY)

if(OCULUS_FOUND AND NOT TARGET Oculus::Oculus)
  add_library(Oculus::Oculus INTERFACE IMPORTED)
  set_target_properties(Oculus::Oculus PROPERTIES
    INTERFACE_LINK_LIBRARIES "${Oculus_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Oculus_INCLUDE_DIR}")
endif()
