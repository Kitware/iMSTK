#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

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

set(H_FILES
  InternalForceModel/imstkCorotationalFemForceModel.h
  InternalForceModel/imstkInternalForceModel.h
  InternalForceModel/imstkInternalForceModelTypes.h
  InternalForceModel/imstkIsotropicHyperelasticFeForceModel.h
  InternalForceModel/imstkLinearFemForceModel.h
  InternalForceModel/imstkStVKForceModel.h
  ObjectModels/imstkAbstractDynamicalModel.h
  ObjectModels/imstkDynamicalModel.h
  ObjectModels/imstkFemDeformableBodyModel.h
  ObjectModels/imstkLevelSetModel.h
  ObjectModels/imstkPbdConstraintFunctor.h
  ObjectModels/imstkPbdModel.h
  ObjectModels/imstkRigidBodyModel2.h
  ObjectModels/imstkSphBoundaryConditions.h
  ObjectModels/imstkSPHKernels.h
  ObjectModels/imstkSphModel.h
  ObjectStates/imstkLevelSetState.h
  ObjectStates/imstkPbdState.h
  ObjectStates/imstkRigidBodyState2.h
  ObjectStates/imstkSphState.h
  ObjectStates/imstkVectorizedState.h
  TimeIntegrators/imstkBackwardEuler.h
  TimeIntegrators/imstkNewmarkBeta.h
  TimeIntegrators/imstkTimeIntegrator.h
  )

set(SRC_FILES
  InternalForceModel/imstkCorotationalFemForceModel.cpp
  InternalForceModel/imstkInternalForceModel.cpp
  InternalForceModel/imstkIsotropicHyperelasticFeForceModel.cpp
  InternalForceModel/imstkLinearFemForceModel.cpp
  InternalForceModel/imstkStVKForceModel.cpp
  ObjectModels/imstkAbstractDynamicalModel.cpp
  ObjectModels/imstkFemDeformableBodyModel.cpp
  ObjectModels/imstkLevelSetModel.cpp
  ObjectModels/imstkPbdModel.cpp
  ObjectModels/imstkRigidBodyModel2.cpp
  ObjectModels/imstkSphBoundaryConditions.cpp
  ObjectModels/imstkSphModel.cpp
  ObjectStates/imstkPbdState.cpp
  ObjectStates/imstkSphState.cpp
  ObjectStates/imstkVectorizedState.cpp
  TimeIntegrators/imstkBackwardEuler.cpp
  )

# If using model reduction link to these libs
if (iMSTK_USE_MODEL_REDUCTION)
  list(APPEND Dependencies
    VegaFEM::reducedStvk  
    VegaFEM::reducedElasticForceModel
    )

  list(APPEND H_FILES ObjectModels/imstkReducedStVKBodyModel.h)
  list(APPEND SRC_FILES ObjectModels/imstkReducedStVKBodyModel.cpp)
endif()

include(imstkAddLibrary)
imstk_add_library(DynamicalModels
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${SRC_FILES}
  DEPENDS
    ${Dependencies}
  )
 
#-----------------------------------------------------------------------------
# Testing and benchmarking
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()

if( ${PROJECT_NAME}_BUILD_BENCHMARK )
  add_subdirectory(Benchmarking)
endif()
