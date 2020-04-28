#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( SceneElements
  DEPENDS
    Common
    Geometry
    GeometryMappers
    MeshIO
    DataStructures
    Materials
    Constraints
    Devices
    DynamicalModels
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()