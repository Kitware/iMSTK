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
/// \struct PbdFemConstraintConfig
///
/// \brief Either mu/lambda used, may be computed from youngs modulus and poissons ratio
///
struct PbdFemConstraintConfig
{
    PbdFemConstraintConfig(double mu, double lambda, double youngModulus, double poissonRatio) :
        m_mu(mu), m_lambda(lambda), m_YoungModulus(youngModulus), m_PoissonRatio(poissonRatio)
    {
    }

    void setYoungAndPoisson(double youngModulus, double poissonRatio)
    {
        m_YoungModulus = youngModulus;
        m_PoissonRatio = poissonRatio;
        const double E  = youngModulus;
        const double nu = poissonRatio;
        m_mu     = E / 2.0 / (1.0 + nu);
        m_lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
    }

    void setMuAndLambda(double mu, double lambda)
    {
        m_mu           = mu;
        m_lambda       = lambda;
        m_YoungModulus = mu * (3.0 * lambda + 2.0 * mu) / (lambda + mu);
        m_PoissonRatio = lambda / 2.0 / (lambda + mu);
    }

    PbdFemConstraintConfig() = default;

    double m_mu     = 0.0;        ///< Lame constant, if constraint type is Fem
    double m_lambda = 0.0;        ///< Lame constant, if constraint type is Fem

    double m_YoungModulus = 1000; ///< Fem parameter, if constraint type is Fem
    double m_PoissonRatio = 0.2;  ///< Fem parameter, if constraint type is Fem
};

///
/// \class PbdFemConstraint
///
/// \brief The PbdFemConstraint class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
/// We provide several model for elastic energy including:
/// Linear, Co-rotation, St Venant-Kirchhof and NeoHookean
///
class PbdFemConstraint : public PbdConstraint
{
public:
    enum class MaterialType
    {
        Linear,
        Corotation,
        StVK,
        NeoHookean
    };

    PbdFemConstraint(const unsigned int cardinality,
                     MaterialType       mType = MaterialType::StVK) : PbdConstraint(cardinality),
        m_initialElementVolume(0.0),
        m_material(mType),
        m_invRestMat(Mat3d::Identity())
    {
    }

    IMSTK_TYPE_NAME(PbdFemConstraint)

public:
    double       m_initialElementVolume = 0.0; ///< Volume of the element
    MaterialType m_material;                   ///< Material type
    Mat3d m_invRestMat;

    PbdFemConstraintConfig m_config;
};
} // namespace imstk