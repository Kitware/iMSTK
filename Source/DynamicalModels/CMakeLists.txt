#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)

imstk_add_library( DynamicalModels
  SUBDIR_LIST
    ObjectStates
    TimeIntegrators
    ObjectModels
    InternalForceModel
  DEPENDS    
    DataStructures
    Constraints
    Geometry
    Solvers
    MeshIO
    VegaFEM::massSpringSystem
    VegaFEM::corotationalLinearFEM
    VegaFEM::isotropicHyperelasticFEM
    VegaFEM::forceModel
    VegaFEM::stvk
    VegaFEM::graph
    VegaFEM::volumetricMesh
    PhysX
  )
 
#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()