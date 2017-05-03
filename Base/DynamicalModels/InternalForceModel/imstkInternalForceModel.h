/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#ifndef imstkInternalForceModel_h
#define imstkInternalForceModel_h

#include "g3log/g3log.hpp"

#include "imstkMath.h"

//vega
#include "forceModel.h"

namespace imstk
{
///
/// \class InternalForceModel
///
/// \brief Base class for internal force model within the following context:
///
///
class InternalForceModel
{
public:
    ///
    /// \brief Constructor
    ///
    InternalForceModel(){};

    ///
    /// \brief Destructor
    ///
    virtual ~InternalForceModel() = default;

    ///
    /// \brief Get the internal force given the present state
    ///
    virtual void getInternalForce(const Vectord& u, Vectord& internalForce) = 0;

    ///
    /// \brief Return the tangent stiffness matrix the present state
    ///
    virtual void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) = 0;

    ///
    /// \brief Return the tangent stiffness matrix the present state
    ///
    virtual void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) = 0;

    ///
    /// \brief Return both internal force and tangent stiffness matrix given the present state
    ///
    virtual void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix) = 0;

    ///
    /// \brief Update the values of the Eigen sparse matrix given the linearized array of data from the Vega matrix
    ///
    static void updateValuesFromMatrix(std::shared_ptr<vega::SparseMatrix> vegaMatrix, double *values);

    ///
    /// \brief
    ///
    virtual void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) = 0;
};
} //imstk

#endif // ifndef imstkTimeIntegrator_h
