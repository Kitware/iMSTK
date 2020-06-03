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

//imstk
#include "imstkInternalForceModel.h"
#include "imstkLogger.h"

//vega
#pragma warning( push )
#pragma warning( disable : 4458 )
#include "StVKInternalForces.h"
#pragma warning( pop )

#include "StVKStiffnessMatrix.h"
#include "StVKElementABCDLoader.h"
#include "tetMesh.h"

namespace imstk
{
class StVKForceModel : public InternalForceModel
{
public:
    ///
    /// \brief Constructor
    ///
    StVKForceModel(std::shared_ptr<vega::VolumetricMesh> mesh,
                   const bool                            withGravity = true,
                   const double                          gravity     = 10.0) : InternalForceModel()
    {
        auto                   tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);
        vega::StVKElementABCD* precomputedIntegrals = vega::StVKElementABCDLoader::load(tetMesh.get());
        m_stVKInternalForces      = std::make_shared<vega::StVKInternalForces>(tetMesh.get(), precomputedIntegrals, withGravity, gravity);
        m_vegaStVKStiffnessMatrix = std::make_shared<vega::StVKStiffnessMatrix>(m_stVKInternalForces.get());
    }

    ///
    /// \brief Destructor
    ///
    virtual ~StVKForceModel() = default;

    ///
    /// \brief Get the internal force
    ///
    inline void getInternalForce(const Vectord& u, Vectord& internalForce) override
    {
        double* data = const_cast<double*>(u.data());
        m_stVKInternalForces->ComputeForces(data, internalForce.data());
    }

    ///
    /// \brief Get the tangent stiffness matrix topology
    ///
    inline void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override
    {
        m_vegaStVKStiffnessMatrix->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
    }

    ///
    /// \brief Set the tangent stiffness matrix
    ///
    inline void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override
    {
        double* data = const_cast<double*>(u.data());
        m_vegaStVKStiffnessMatrix->ComputeStiffnessMatrix(data, m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    ///
    /// \brief Set the tangent stiffness matrix and internal force
    ///
    inline void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix) override
    {
        getInternalForce(u, internalForce);
        getTangentStiffnessMatrix(u, tangentStiffnessMatrix);
    }

    ///
    /// \brief Speficy tangent stiffness matrix
    ///
    inline void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override
    {
        m_vegaTangentStiffnessMatrix = K;
    }

protected:

    std::shared_ptr<vega::StVKInternalForces>  m_stVKInternalForces;
    std::shared_ptr<vega::SparseMatrix>        m_vegaTangentStiffnessMatrix;
    std::shared_ptr<vega::StVKStiffnessMatrix> m_vegaStVKStiffnessMatrix;
    bool ownStiffnessMatrix;
};
} // imstk
