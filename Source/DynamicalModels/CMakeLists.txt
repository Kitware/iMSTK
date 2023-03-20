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
  ObjectModels/imstkAbstractDynamicalSystem.h
  ObjectModels/imstkLevelSetSystem.h
  ObjectModels/imstkPbdConstraintFunctor.h
  ObjectModels/imstkPbdSystem.h
  ObjectModels/imstkPbdSystemConfig.h
  ObjectModels/imstkSphBoundaryConditions.h
  ObjectModels/imstkSPHKernels.h
  ObjectModels/imstkSphSystem.h
  ObjectStates/imstkSphState.h
  )

set(SRC_FILES
  ObjectModels/imstkAbstractDynamicalSystem.cpp
  ObjectModels/imstkLevelSetSystem.cpp
  ObjectModels/imstkPbdSystem.cpp
  ObjectModels/imstkPbdSystemConfig.cpp
  ObjectModels/imstkSphBoundaryConditions.cpp
  ObjectModels/imstkSphSystem.cpp
  ObjectStates/imstkSphState.cpp
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
