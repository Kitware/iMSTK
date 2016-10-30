#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( DynamicalModels
  DEPENDS
    Core
    Geometry
    Constraint
    TimeIntegrators
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
