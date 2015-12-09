
simmedtk_add_library(Solvers
  SOURCES
    SolverBase.cpp
	LinearSolver.cpp
	IterativeLinearSolver.cpp
	DirectLinearSolver.cpp
	ConjugateGradient.cpp
	GaussSeidelSolver.cpp
  PUBLIC_HEADERS
	SystemOfEquations.h
    SolverBase.h
	LinearSolver.h
	IterativeLinearSolver.h
	DirectLinearSolver.h
	ConjugateGradient.h
	GaussSeidelSolver.h
)

target_link_libraries(Solvers
  PUBLIC
    Core
  PRIVATE
	VegaFEM::sparseSolver
    VegaFEM::sparseMatrix
    Event
	Assembler
)
