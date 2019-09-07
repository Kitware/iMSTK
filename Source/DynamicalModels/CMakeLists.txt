#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( DynamicalModels
  SUBDIR_LIST
    ObjectModels
    ObjectModels/PbdConstraints
    ObjectStates
    InternalForceModel
  DEPENDS
    Core
    Constraints
    Geometry
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
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
