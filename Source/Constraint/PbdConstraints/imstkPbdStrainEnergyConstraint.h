/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \struct PbdStrainEnergyConstraintConfig
///
/// \brief Either mu/lambda used, may be computed from youngs modulus and poissons ratio
///
struct PbdStrainEnergyConstraintConfig
{
    PbdStrainEnergyConstraintConfig(double mu, double lambda, double youngModulus, double poissonRatio) :
        m_mu(mu), m_lambda(lambda), m_YoungModulus(youngModulus), m_PoissonRatio(poissonRatio)
    {
    }

    double m_mu     = 0.0;        ///< Lame constant, if constraint type is Fem
    double m_lambda = 0.0;        ///< Lame constant, if constraint type is Fem

    double m_YoungModulus = 1000; ///< Fem parameter, if constraint type is Fem
    double m_PoissonRatio = 0.2;  ///< Fem parameter, if constraint type is Fem
};

///
/// \class PbdStrainEnergyConstraint
///
/// \brief The PbdStrainEnergyConstraint class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
/// We provide several model for elastic energy including:
/// Linear, Co-rotation, St Venant-Kirchhof and NeoHookean
///
class PbdStrainEnergyConstraint : public PbdConstraint
{
public:
    enum class MaterialType
    {
        Linear,
        Corotation,
        StVK,
        NeoHookean
    };

    PbdStrainEnergyConstraint(const unsigned int cardinality,
                              MaterialType       mType = MaterialType::StVK) : PbdConstraint(cardinality),
        m_initialElementVolume(0.0),
        m_material(mType),
        m_invRestMat(Mat3d::Identity())
    {
    }

public:
    double       m_initialElementVolume = 0.0; ///< Volume of the element
    MaterialType m_material;                   ///< Material type
    Mat3d m_invRestMat;

    std::shared_ptr<PbdStrainEnergyConstraintConfig> m_config = nullptr;
};
} // namespace imstk