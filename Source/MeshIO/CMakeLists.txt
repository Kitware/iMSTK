#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( MeshIO
  DEPENDS
    Common
	Geometry    
    Assimp    
    VegaFEM::volumetricMesh
    ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
    add_subdirectory("Testing")
endif()
