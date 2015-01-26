
if( MSVC )
  # in order to prevent DLL hell, each of the DLLs have to be suffixed with the major version and msvc prefix
  if( MSVC70 OR MSVC71 )
    set(MSVC_PREFIX "vc70")
  elseif( MSVC80 )
    set(MSVC_PREFIX "vc80")
  elseif( MSVC90 )
    set(MSVC_PREFIX "vc90")
  else()
    set(MSVC_PREFIX "vc100")
  endif()
  set(LIBRARY_SUFFIX "-${MSVC_PREFIX}-mt" CACHE STRING "the suffix for the assimp windows library" FORCE)
endif()


find_path(ASSIMP_INCLUDE_DIR
    assimp/config.h
    )

find_library(ASSIMP_LIBRARY
  NAMES
    assimp${LIBRARY_SUFFIX}
    assimp${LIBRARY_SUFFIX}d
    )

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

if(ASSIMP_FOUND AND NOT TARGET Assimp::Assimp)
  add_library(Assimp::Assimp INTERFACE IMPORTED)
  set_target_properties(Assimp::Assimp PROPERTIES
    INTERFACE_LINK_LIBRARIES "${ASSIMP_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIR}")
endif()
