#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)

list(APPEND Dependencies
  Common
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
  )
list(APPEND ExclusionFiles "")

# If using model reduction link to these libs
if (iMSTK_USE_MODEL_REDUCTION)
  list(APPEND Dependencies
    VegaFEM::reducedStvk  
    VegaFEM::reducedElasticForceModel)
else()
  list(APPEND ExclusionFiles
    ObjectModels/imstkReducedStVKBodyModel.h
    ObjectModels/imstkReducedStVKBodyModel.cpp)
endif()

if (iMSTK_USE_PHYSX)
  list(APPEND Dependencies PhysX::PhysX)
else()
  list(APPEND ExclusionFiles
    ObjectModels/imstkRigidBodyModel.h
    ObjectModels/imstkRigidBodyModel.cpp
    ObjectModels/imstkRigidBodyState.h
    ObjectModels/imstkRigidBodyState.cpp
    ObjectModels/imstkRigidBodyWorld.h
    ObjectModels/imstkRigidBodyWorld.cpp
    )
endif()


imstk_add_library(DynamicalModels
  SUBDIR_LIST
    ObjectStates
    TimeIntegrators
    ObjectModels
    InternalForceModel
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS
    ${Dependencies})
 
#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()

if( ${PROJECT_NAME}_BUILD_BENCHMARK )
  add_subdirectory(Benchmarking)
  add_subdirectory(BenchmarkView)
endif()
