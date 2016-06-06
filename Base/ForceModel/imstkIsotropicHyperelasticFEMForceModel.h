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

//vega
#include "isotropicHyperelasticFEM.h"
#include "StVKIsotropicMaterial.h"
#include "neoHookeanIsotropicMaterial.h"
#include "MooneyRivlinIsotropicMaterial.h"

namespace imstk
{

class IsotropicHyperelasticFEForceModel : virtual public InternalForceModel
{

    enum class hyperElasticMaterialType
    {
        StVK,
        NeoHookean,
        MooneyRivlin
    };

public:
    IsotropicHyperelasticFEForceModel(hyperElasticMaterialType materialType, std::shared_ptr<vega::VolumetricMesh> mesh, double inversionThreshold, bool withGravity = true, double gravity = 10.0)
    {
        int enableCompressionResistance = 1;
        double compressionResistance = 500;
        switch (materialType)
        {
            case hyperElasticMaterialType::StVK:
                m_isotropicMaterial = std::make_shared<vega::StVKIsotropicMaterial>(mesh.get(), enableCompressionResistance, compressionResistance);
                break;

            case hyperElasticMaterialType::NeoHookean:
                m_isotropicMaterial = std::make_shared<vega::NeoHookeanIsotropicMaterial>(mesh.get(), enableCompressionResistance, compressionResistance);
                break;

            case hyperElasticMaterialType::MooneyRivlin:
                m_isotropicMaterial = std::make_shared<vega::MooneyRivlinIsotropicMaterial>(mesh.get(), enableCompressionResistance, compressionResistance);
                break;

            default:
                LOG(ERROR) << "Error: Invalid hyperelastic material type.";
        }

        m_isotropicHyperelasticFEM = std::make_shared<vega::IsotropicHyperelasticFEM>(mesh.get(), m_isotropicMaterial, inversionThreshold, withGravity, gravity);
    }

    virtual ~IsotropicHyperelasticFEForceModel();

    void getInternalForce(Vectord& u, Vectord& internalForce)
    {
        m_isotropicHyperelasticFEM->ComputeForces(u.data(), internalForce.data());
    }

    void getTangentStiffnessMatrix(Vectord& u, std::shared_ptr<SparseMatrixd> tangentStiffnessMatrix)
    {
        m_isotropicHyperelasticFEM->GetTangentStiffnessMatrix(u.data(), m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix->valuePtr());
    }

    void GetForceAndMatrix(Vectord& u, Vectord& internalForce, std::shared_ptr<SparseMatrixd> tangentStiffnessMatrix)
    {
        m_isotropicHyperelasticFEM->GetForceAndTangentStiffnessMatrix(u.data(), internalForce.data(), m_vegaTangentStiffnessMatrix.get());
        InternalForceModel::updateValuesFromMatrix(m_vegaTangentStiffnessMatrix, tangentStiffnessMatrix->valuePtr());
    }

protected:
    std::shared_ptr<vega::IsotropicHyperelasticFEM> m_isotropicHyperelasticFEM;
    std::shared_ptr<vega::IsotropicMaterial> m_isotropicMaterial;
    std::shared_ptr<vega::SparseMatrix> m_vegaTangentStiffnessMatrix;
};

} // imstk

#endif // imstkHyperElasticFEMForceModel_h
