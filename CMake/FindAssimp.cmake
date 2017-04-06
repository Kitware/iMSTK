#-----------------------------------------------------------------------------
# Find path
#-----------------------------------------------------------------------------
find_path(Assimp_INCLUDE_DIRS
  NAMES
    assimp/Importer.hpp
    )
mark_as_advanced(Assimp_INCLUDE_DIRS)

#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(Assimp_LIBRARIES
  NAMES
    libassimp
    assimp
    libassimpd
    assimpd
  )
mark_as_advanced(Assimp_LIBRARIES)

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Assimp
  REQUIRED_VARS
    Assimp_INCLUDE_DIRS
    Assimp_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(Assimp_FOUND AND NOT TARGET Assimp)
  add_library(Assimp INTERFACE IMPORTED)
  set_target_properties(Assimp PROPERTIES
    INTERFACE_LINK_LIBRARIES "${Assimp_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${Assimp_INCLUDE_DIRS}"
  )
endif()
