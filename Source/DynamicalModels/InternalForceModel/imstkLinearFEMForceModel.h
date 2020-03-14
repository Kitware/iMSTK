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

#include <memory>

#include "g3log/g3log.hpp"

//imstk
#include "imstkInternalForceModel.h"

//vega
#include "StVKInternalForces.h"
#include "StVKStiffnessMatrix.h"
#include "StVKElementABCDLoader.h"

namespace imstk
{
///
/// \class LinearFEMForceModel
///
/// \brief Force model for linear finite element formulation
///
class LinearFEMForceModel : public InternalForceModel
{
public:
    ///
    /// \brief Constructor
    ///
    LinearFEMForceModel(std::shared_ptr<vega::VolumetricMesh> mesh,
                        const bool                            withGravity = true,
                        const double                          gravity     = -9.81) : InternalForceModel()
    {
        auto tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);

        vega::StVKElementABCD* precomputedIntegrals = vega::StVKElementABCDLoader::load(tetMesh.get());
        m_stVKInternalForces = std::make_shared<vega::StVKInternalForces>(tetMesh.get(), precomputedIntegrals, withGravity, gravity);

        auto stVKStiffnessMatrix = std::make_shared<vega::StVKStiffnessMatrix>(m_stVKInternalForces.get());

        stVKStiffnessMatrix->GetStiffnessMatrixTopology(&m_stiffnessMatrixRawPtr);
        std::shared_ptr<vega::SparseMatrix> m_stiffnessMatrix2(m_stiffnessMatrixRawPtr);
        m_stiffnessMatrix = m_stiffnessMatrix2;

        auto K = m_stiffnessMatrix.get();
        stVKStiffnessMatrix->GetStiffnessMatrixTopology(&K);
        double* zero = (double*)calloc(m_stiffnessMatrix->GetNumRows(), sizeof(double));
        stVKStiffnessMatrix->ComputeStiffnessMatrix(zero, m_stiffnessMatrix.get());
        free(zero);
    };

    ///
    /// \brief Destructor
    ///
    virtual ~LinearFEMForceModel()
    {
        if (m_stiffnessMatrixRawPtr)
        {
            delete m_stiffnessMatrixRawPtr;
        }
    };

    ///
    /// \brief Get the internal force
    ///
    inline void getInternalForce(const Vectord& u, Vectord& internalForce) override
    {
        double* data = const_cast<double*>(u.data());
        m_stiffnessMatrix->MultiplyVector(data, internalForce.data());
    }

    ///
    /// \brief Get the tangent stiffness matrix
    ///
    inline void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override
    {
        InternalForceModel::updateValuesFromMatrix(m_stiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    ///
    /// \brief Get the tangent stiffness matrix topology
    ///
    inline void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override
    {
        *tangentStiffnessMatrix = new vega::SparseMatrix(*m_stiffnessMatrix.get());
    }

    ///
    /// \brief Get the tangent stiffness matrix and internal force
    ///
    inline void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix) override
    {
        getInternalForce(u, internalForce);
        getTangentStiffnessMatrix(u, tangentStiffnessMatrix);
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
} // imstk
