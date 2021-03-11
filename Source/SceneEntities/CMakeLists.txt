#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

if (NOT iMSTK_USE_MODEL_REDUCTION)
  list(APPEND ExclusionFiles
    Objects/imstkReducedFeDeformableObject.h
    Objects/imstkReducedFeDeformableObject.cpp)
endif()

include(imstkAddLibrary)
imstk_add_library(SceneEntities
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS    
    Common
    Geometry
    GeometryMappers
    Materials
    MeshIO
    Constraints
    Devices
    DynamicalModels)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()