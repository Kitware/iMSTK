#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Constraints
  SUBDIR_LIST
    PbdConstraints
    RigidBodyConstraints
  DEPENDS
    Common
    CollisionDetection
    DataStructures
    Geometry
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
