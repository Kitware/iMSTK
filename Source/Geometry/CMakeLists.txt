#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Geometry
  DEPENDS
    Core
    Datastructures
    glm
    Assimp
    Materials
    VegaFEM::volumetricMesh
    ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( BUILD_TESTING )
  include(imstkAddTest)
  imstk_add_test( Geometry )
endif()
