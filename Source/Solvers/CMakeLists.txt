#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Solvers
  H_FILES
    imstkConjugateGradient.h
    imstkDirectLinearSolver.h
    imstkGaussSeidel.h
    imstkIterativeLinearSolver.h
    imstkJacobi.h
    imstkLinearSolver.h
    imstkNewtonSolver.h
    imstkNonLinearSolver.h
    imstkNonLinearSystem.h
    imstkPbdSolver.h
    imstkProjectedGaussSeidelSolver.h
    imstkSolverBase.h
    imstkSOR.h
  CPP_FILES
    imstkConjugateGradient.cpp
    imstkDirectLinearSolver.cpp
    imstkGaussSeidel.cpp
    imstkIterativeLinearSolver.cpp
    imstkJacobi.cpp
    imstkNewtonSolver.cpp
    imstkNonLinearSolver.cpp
    imstkNonLinearSystem.cpp
    imstkPbdSolver.cpp
    imstkSOR.cpp
  DEPENDS
    Constraints
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()
