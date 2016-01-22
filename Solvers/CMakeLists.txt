set(MODULE_NAME Solvers)
simmedtk_add_library(${MODULE_NAME}
  SOURCES
    BackwardGaussSeidel.cpp
    BackwardSOR.cpp
    BackwardSOR.cpp
    ConjugateGradient.cpp
    DirectLinearSolver.cpp
    ForwardGaussSeidel.cpp
    ForwardSOR.cpp
    NewtonMethod.cpp
    IterativeLinearSolver.cpp
    NonLinearSolver.cpp
  PUBLIC_HEADERS
    BackwardGaussSeidel.h
    BackwardSOR.h
    BackwardSOR.h
    ConjugateGradient.h
    DirectLinearSolver.h
    ForwardGaussSeidel.h
    ForwardSOR.h
    NewtonMethod.h
    IterativeLinearSolver.h
    LinearSolver.h
    LinearSolver.hpp
    NonLinearSolver.h
    SystemOfEquations.h
)

if(UNIX)
  target_compile_features(${MODULE_NAME} PUBLIC cxx_auto_type)
endif()

if(BUILD_TESTING)
    add_subdirectory(UnitTests)
endif(BUILD_TESTING)
