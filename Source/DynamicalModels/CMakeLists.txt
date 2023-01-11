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
  )

set(H_FILES
  ObjectModels/imstkAbstractDynamicalModel.h
  ObjectModels/imstkDynamicalModel.h
  ObjectModels/imstkLevelSetModel.h
  ObjectModels/imstkPbdConstraintFunctor.h
  ObjectModels/imstkPbdModel.h
  ObjectModels/imstkPbdModelConfig.h
  ObjectModels/imstkRigidBodyModel2.h
  ObjectModels/imstkSphBoundaryConditions.h
  ObjectModels/imstkSPHKernels.h
  ObjectModels/imstkSphModel.h
  ObjectStates/imstkRigidBodyState2.h
  ObjectStates/imstkSphState.h
  ObjectStates/imstkVectorizedState.h
  TimeIntegrators/imstkBackwardEuler.h
  TimeIntegrators/imstkNewmarkBeta.h
  TimeIntegrators/imstkTimeIntegrator.h
  )

set(SRC_FILES
  ObjectModels/imstkAbstractDynamicalModel.cpp
  ObjectModels/imstkLevelSetModel.cpp
  ObjectModels/imstkPbdModel.cpp
  ObjectModels/imstkPbdModelConfig.cpp
  ObjectModels/imstkRigidBodyModel2.cpp
  ObjectModels/imstkSphBoundaryConditions.cpp
  ObjectModels/imstkSphModel.cpp
  ObjectStates/imstkSphState.cpp
  ObjectStates/imstkVectorizedState.cpp
  TimeIntegrators/imstkBackwardEuler.cpp
  )

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
