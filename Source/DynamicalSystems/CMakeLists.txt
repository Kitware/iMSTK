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
  imstkAbstractDynamicalSystem.h
  imstkLevelSetSystem.h
  imstkPbdConstraintFunctor.h
  imstkPbdState.h
  imstkPbdSystem.h
  imstkPbdSystemConfig.h
  imstkSphBoundaryConditions.h
  imstkSPHKernels.h
  imstkSphSystem.h
  imstkSphState.h
  )

set(SRC_FILES
  imstkAbstractDynamicalSystem.cpp
  imstkLevelSetSystem.cpp
  imstkPbdSystem.cpp
  imstkPbdSystemConfig.cpp
  imstkSphBoundaryConditions.cpp
  imstkSphSystem.cpp
  imstkSphState.cpp
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
