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

#ifndef imstkHyperElasticFEMForceModel_h
#define imstkHyperElasticFEMForceModel_h

#include <g3log/g3log.hpp>

#include "imstkInternalForceModel.h"
#include "imstkForceModelConfig.h"

//vega
#include "isotropicHyperelasticFEM.h"
#include "StVKIsotropicMaterial.h"
#include "neoHookeanIsotropicMaterial.h"
#include "MooneyRivlinIsotropicMaterial.h"

namespace imstk
{
class IsotropicHyperelasticFEForceModel : public InternalForceModel
{

public:
    IsotropicHyperelasticFEForceModel(HyperElasticMaterialType materialType,
        std::shared_ptr<vega::VolumetricMesh> mesh,
        double inversionThreshold,
        bool withGravity = true,
        double gravity = 10.0) : InternalForceModel()
    {
        auto tetMesh = std::dynamic_pointer_cast<vega::TetMesh>(mesh);

        int enableCompressionResistance = 1;
        double compressionResistance = 500;
        switch (materialType)
        {
            case HyperElasticMaterialType::StVK:
                m_isotropicMaterial = std::make_shared<vega::StVKIsotropicMaterial>(
                    tetMesh.get(),
                    enableCompressionResistance,
                    compressionResistance);
                break;

            case HyperElasticMaterialType::NeoHookean:
                m_isotropicMaterial = std::make_shared<vega::NeoHookeanIsotropicMaterial>(
                    tetMesh.get(),
                    enableCompressionResistance,
                    compressionResistance);
                break;

            case HyperElasticMaterialType::MooneyRivlin:
                m_isotropicMaterial = std::make_shared<vega::MooneyRivlinIsotropicMaterial>(
                    tetMesh.get(),
                    enableCompressionResistance,
                    compressionResistance);
                break;

            default:
                LOG(WARNING) << "Error: Invalid hyperelastic material type.";
        }

        m_isotropicHyperelasticFEM = std::make_shared<vega::IsotropicHyperelasticFEM>(
            tetMesh.get(),
            m_isotropicMaterial.get(),
            inversionThreshold,
            withGravity,
            gravity);
    }

    IsotropicHyperelasticFEForceModel() = delete;

    virtual ~IsotropicHyperelasticFEForceModel(){};

    void getInternalForce(const Vectord& u, Vectord& internalForce)
    {
        double *data = const_cast<double*>(u.data());
        m_isotropicHyperelasticFEM->ComputeForces(data, internalForce.data());
    }

    void getTangentStiffnessMatrix(const Vectord& u, SparseMatrixd& tangentStiffnessMatrix)
    {
        double *data = const_cast<double*>(u.data());
        m_isotropicHyperelasticFEM->GetTangentStiffnessMatrix(data, m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    virtual void getTangentStiffnessMatrixTopology(vega::SparseMatrix** tangentStiffnessMatrix)
    {
        m_isotropicHyperelasticFEM->GetStiffnessMatrixTopology(tangentStiffnessMatrix);
    }

    void GetForceAndMatrix(const Vectord& u, Vectord& internalForce, SparseMatrixd& tangentStiffnessMatrix)
    {
        double *data = const_cast<double*>(u.data());
        m_isotropicHyperelasticFEM->GetForceAndTangentStiffnessMatrix(data, internalForce.data(), m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix.valuePtr());
    }

    virtual void setTangentStiffness(std::shared_ptr<vega::SparseMatrix> K) override
    {
        m_vegaTangentStiffnessMatrix = K;
    }

protected:
    std::shared_ptr<vega::IsotropicHyperelasticFEM> m_isotropicHyperelasticFEM;
    std::shared_ptr<vega::IsotropicMaterial> m_isotropicMaterial;
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;
};

} // imstk

#endif // imstkHyperElasticFEMForceModel_h
