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

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4458 )
#endif
#include <isotropicHyperelasticFEM.h>
#ifdef WIN32
#pragma warning( pop )
#endif

namespace imstk
{
///
/// \class IsotropicHyperelasticFeForceModel
///
/// \brief Force model for the isotropic hyperelastic material
///
class IsotropicHyperelasticFeForceModel : public InternalForceModel
{
public:
    IsotropicHyperelasticFeForceModel(const HyperElasticMaterialType        materialType,
                                      std::shared_ptr<vega::VolumetricMesh> mesh,
                                      const double                          inversionThreshold,
                                      const bool                            withGravity = true,
                                      const double                          gravity     = 10.0);
    IsotropicHyperelasticFeForceModel() = delete;
    ~IsotropicHyperelasticFeForceModel() override = default;

    ///
    /// \brief Get the internal force
    ///
    inline void getInternalForce(const Vectord& u, Vectord& internalForce) override
    {
        double* data = const_cast<double*>(u.data());
        m_isotropicHyperelasticFem->ComputeForces(data, internalForce.data());
    }

    ///
    /// \brief Get the tangent stiffness matrix
    ///
    inline void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix) override
    {
        double* data = const_cast<double*>(u.data());
        m_isotropicHyperelasticFem->GetTangentStiffnessMatrix(data, m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    ///
    /// \brief Get the tangent stiffness matrix topology
    ///
    inline void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix) override
    {
        m_isotropicHyperelasticFem->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
    }

    ///
    /// \brief Get the tangent stiffness matrix and internal force
    ///
    inline void getForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix) override
    {
        double* data = const_cast<double*>(u.data());
        m_isotropicHyperelasticFem->GetForceAndTangentStiffnessMatrix(data, internalForce.data(), m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    ///
    /// \brief Set the tangent stiffness matrix
    ///
    inline void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override
    {
        m_vegaTangentStiffnessMatrix = K;
    }

protected:
    std::shared_ptr<vega::IsotropicHyperelasticFEM> m_isotropicHyperelasticFem; ///>
    std::shared_ptr<vega::IsotropicMaterial> m_isotropicMaterial;               ///>
    std::shared_ptr<vega::SparseMatrix>      m_vegaTangentStiffnessMatrix;      ///>
};
} // namespace imstk