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

#pragma once

#include "imstkLogger.h"

#include "imstkInternalForceModel.h"

#pragma warning( push )
#pragma warning( disable : 4458 )
//vega
#include "massSpringSystem.h"
#pragma warning( pop )

namespace imstk
{
///
/// \class MassSpringForceModel
///
/// \brief Mathematical model for the MassSpring
///
class MassSpringForceModel : public InternalForceModel
{
public:
    ///
    /// \brief Constructor
    ///
    explicit MassSpringForceModel(std::shared_ptr<vega::MassSpringSystem> massSpringSystem);
    MassSpringForceModel() = delete;

    ///
    /// \brief Destructor
    ///
    virtual ~MassSpringForceModel() = default;

    ///
    /// \brief Set the internal force
    ///
    void getInternalForce(const Vectord& u, Vectord& internalForce) override;

    ///
    /// \brief Set the tangent stiffness matrix topology
    ///
    virtual void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override;

    ///
    /// \brief Set the tangent stiffness matrix
    ///
    void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override;

protected:
    std::shared_ptr<vega::MassSpringSystem> m_massSpringSystem;// Need to be initialized
    std::shared_ptr<vega::SparseMatrix>     m_vegaTangentStiffnessMatrix;
};
} // imstk
