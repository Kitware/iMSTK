#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( MeshIO
  DEPENDS
    Common
	Geometry
    glm
    Assimp    
    VegaFEM::volumetricMesh
    ${VTK_LIBRARIES}
  )
  
target_compile_options(MeshIO PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  include(imstkAddTest)
#  imstk_add_test( Geometry )
#endif()
