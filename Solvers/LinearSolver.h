// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------
#ifndef SM_LINEAR_SOLVER
#define SM_LINEAR_SOLVER

// simmedtk includes
#include "SolverBase/SolverBase.h"

// vega includes
#include "sparseMatrix.h"

///
/// \brief Base class for linear solver
///
class LinearSolver: public SolverBase
{
public:

    ///
    /// \brief default constructor
    ///
    LinearSolver(const std::shared_ptr<SparseLinearSystem> linSys = nullptr);

    ///
    /// \brief destructor
    ///
    ~LinearSolver();

    // -------------------------------------------------
    //  getters
    // -------------------------------------------------

	///
	/// \brief Return the size of the linear system
	///
    int getSystemSize() const;

	///
	/// \brief Get the unknown vector
	///
    virtual Eigen::Vector<double>& getUnknownVector() = 0;

	///
	/// \brief Get the force vector on the r.h.s
	///
    virtual Eigen::Vector<double>& getForceVector() = 0;

    // -------------------------------------------------
    //  setters
    // -------------------------------------------------

    ///
    /// \brief set the Eigen force vector
    ///
    void setForceVector(Eigen::Vector<double>& f);

    ///
    /// \brief set the Eigen unknown vector
    ///
    void setUnknownVector(Eigen::Vector<double>& x);

    ///
    /// \brief Set the linear system of equations based on Eigen
    ///
    void setLinearSystem(const std::shared_ptr<SparseLinearSystem> linSys);

};

#endif // SM_LINEAR_SOLVER
