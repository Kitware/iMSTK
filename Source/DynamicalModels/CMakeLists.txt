#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( DynamicalModels
  SUBDIR_LIST
    ObjectStates
	TimeIntegrators
    ObjectModels
    ObjectModels/PbdConstraints
    InternalForceModel
  DEPENDS
    Core
    Geometry
    Constraints
    Solvers
    VegaFEM::massSpringSystem
    VegaFEM::corotationalLinearFEM
    VegaFEM::isotropicHyperelasticFEM
    VegaFEM::forceModel
    VegaFEM::stvk
    VegaFEM::graph
    VegaFEM::volumetricMesh
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif() 
