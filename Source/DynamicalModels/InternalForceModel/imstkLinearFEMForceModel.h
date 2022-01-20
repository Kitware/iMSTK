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

#include "imstkInternalForceModel.h"

namespace vega
{
class StVKInternalForces;
class VolumetricMesh;
}

namespace imstk
{
///
/// \class LinearFemForceModel
///
/// \brief Force model for linear finite element formulation
///
class LinearFemForceModel : public InternalForceModel
{
public:
    LinearFemForceModel(std::shared_ptr<vega::VolumetricMesh> mesh,
                        const bool withGravity = true, const double gravity = -9.81);
    ~LinearFemForceModel() override;

    ///
    /// \brief Compute the internal force
    ///
    inline void getInternalForce(const Vectord& u, Vectord& internalForce) override
    {
        double* data = const_cast<double*>(u.data());
        m_stiffnessMatrix->MultiplyVector(data, internalForce.data());
    }

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif
    ///
    /// \brief Get the tangent stiffness matrix
    /// \todo Clear warning C4100
    ///
    inline void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override
    {
        InternalForceModel::updateValuesFromMatrix(m_stiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

#ifdef WIN32
#pragma warning( pop )
#endif

    ///
    /// \brief Get the tangent stiffness matrix topology
    ///
    inline void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override
    {
        *tangentStiffnessMatrix = new vega::SparseMatrix(*m_stiffnessMatrix.get());
    }

    ///
    /// \brief Set the tangent stiffness matrix
    ///
    inline void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override
    {
        m_stiffnessMatrix = K;
    }

protected:
    std::shared_ptr<vega::SparseMatrix>       m_stiffnessMatrix;
    std::shared_ptr<vega::StVKInternalForces> m_stVKInternalForces;

    // tmp
    vega::SparseMatrix* m_stiffnessMatrixRawPtr;
};
}