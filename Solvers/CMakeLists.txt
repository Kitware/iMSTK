###########################################################################
#
# Copyright (c) Kitware, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

set(MODULE_NAME Solvers)
imstk_add_library(${MODULE_NAME}
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
