/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdModel.h"
#include "imstkGraph.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkParallelUtils.h"
#include "imstkPbdConstraintFunctor.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdSolver.h"
#include "imstkTaskGraph.h"

#include <algorithm>

namespace
{
template<class FunctorType>
void
eraseOldFunctor(std::vector<std::shared_ptr<imstk::PbdConstraintFunctor>>& funcs, int bodyId)
{
    // Find and erase the functor with the same bodyId
    funcs.erase(std::remove_if(funcs.begin(), funcs.end(), [bodyId](auto item) {
            if (FunctorType* functor = dynamic_cast<FunctorType*>(item.get()))
            {
                return functor->m_bodyIndex == bodyId;
            }
            return false;
                        }), funcs.end());
}
} // namespace

namespace imstk
{
void
PbdModelConfig::computeElasticConstants()
{
    if (std::abs(m_femParams->m_mu) < std::numeric_limits<double>::min()
        && std::abs(m_femParams->m_lambda) < std::numeric_limits<double>::min())
    {
        const double E  = m_femParams->m_YoungModulus;
        const double nu = m_femParams->m_PoissonRatio;
        m_femParams->m_mu     = E / 2.0 / (1.0 + nu);
        m_femParams->m_lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
    }
    else
    {
        const double mu     = m_femParams->m_mu;
        const double lambda = m_femParams->m_lambda;
        m_femParams->m_YoungModulus = mu * (3.0 * lambda + 2.0 * mu) / (lambda + mu);
        m_femParams->m_PoissonRatio = lambda / 2.0 / (lambda + mu);
    }
}

void
PbdModelConfig::enableConstraint(ConstraintGenType type, double stiffness, const int bodyId)
{
    auto& funcs = m_functors[type];

    eraseOldFunctor<PbdBodyConstraintFunctor>(funcs, bodyId);

    if (type == ConstraintGenType::Distance)
    {
        auto functor = std::make_shared<PbdDistanceConstraintFunctor>();
        funcs.push_back(functor);
        functor->setStiffness(stiffness);
        functor->setBodyIndex(bodyId);
    }
    else if (type == ConstraintGenType::Volume)
    {
        auto functor = std::make_shared<PbdVolumeConstraintFunctor>();
        funcs.push_back(functor);
        functor->setStiffness(stiffness);
        functor->setBodyIndex(bodyId);
    }
    else if (type == ConstraintGenType::Area)
    {
        auto functor = std::make_shared<PbdAreaConstraintFunctor>();
        funcs.push_back(functor);
        functor->setStiffness(stiffness);
        functor->setBodyIndex(bodyId);
    }
    else if (type == ConstraintGenType::Bend)
    {
        auto functor = std::make_shared<PbdBendConstraintFunctor>();
        funcs.push_back(functor);
        functor->setStiffness(stiffness);
        functor->setStride(1);
        functor->setBodyIndex(bodyId);
    }
    else if (type == ConstraintGenType::Dihedral)
    {
        auto functor = std::make_shared<PbdDihedralConstraintFunctor>();
        funcs.push_back(functor);
        functor->setStiffness(stiffness);
        functor->setBodyIndex(bodyId);
    }
    else if (type == ConstraintGenType::ConstantDensity)
    {
        auto functor = std::make_shared<PbdConstantDensityConstraintFunctor>();
        funcs.push_back(functor);
        functor->setStiffness(stiffness);
        functor->setBodyIndex(bodyId);
    }
    else
    {
        LOG(FATAL) << "There exists no standard constraint functor for the ConstraintGenType";
    }
}

void
PbdModelConfig::enableDistanceConstraint(const double stiffness, const double stretch, const int bodyId)
{
    auto& funcs = m_functors[ConstraintGenType::Distance];

    eraseOldFunctor<PbdBodyConstraintFunctor>(funcs, bodyId);

    auto functor = std::make_shared<PbdDistanceConstraintFunctor>();
    funcs.push_back(functor);
    functor->setStiffness(stiffness);
    functor->setStretch(stretch);
    functor->setBodyIndex(bodyId);
}

void
PbdModelConfig::enableBendConstraint(const double stiffness, const int stride, const bool restLength0, const int bodyId)
{
    using FunctorType = PbdBendConstraintFunctor;
    auto& funcs = m_functors[ConstraintGenType::Bend];

    // Find and remove the functor with the same bodyId and stride
    funcs.erase(std::remove_if(funcs.begin(), funcs.end(), [bodyId, stride](auto item) {
            if (FunctorType* functor = dynamic_cast<FunctorType*>(item.get()))
            {
                return functor->m_bodyIndex == bodyId && functor->getStride() == stride;
            }
            return false;
                }), funcs.end());

    auto functor = std::make_shared<FunctorType>();

    functor->setRestLength(restLength0 ? 0.0 : -1.0);
    functor->setBodyIndex(bodyId);
    functor->setStiffness(stiffness);
    functor->setStride(stride);
    funcs.push_back(functor);
}

void
PbdModelConfig::enableConstantDensityConstraint(const double stiffness,
                                                const double particleRadius, const double restDensity, const int bodyId)
{
    using FunctorType = PbdConstantDensityConstraintFunctor;
    auto& funcs = m_functors[ConstraintGenType::ConstantDensity];

    eraseOldFunctor<FunctorType>(funcs, bodyId);

    auto functor = std::make_shared<FunctorType>();

    functor->setParticleRadius(particleRadius);
    functor->setBodyIndex(bodyId);
    functor->setStiffness(stiffness);
    functor->setRestDensity(restDensity);

    funcs.push_back(std::move(functor));
}

void
PbdModelConfig::enableFemConstraint(PbdFemConstraint::MaterialType material, const int bodyId)
{
    using FunctorType = PbdFemTetConstraintFunctor;
    auto& funcs = m_functors[ConstraintGenType::FemTet];

    eraseOldFunctor<FunctorType>(funcs, bodyId);

    auto functor = std::make_shared<FunctorType>();

    functor->setBodyIndex(bodyId);
    functor->setFemConfig(m_femParams);
    functor->setMaterialType(material);

    funcs.push_back(std::move(functor));
}

void
PbdModelConfig::enableFemConstraint(PbdFemConstraint::MaterialType material, double youngsModulus, double poisson, const int bodyId)
{
    using FunctorType = PbdFemTetConstraintFunctor;
    auto& funcs = m_functors[ConstraintGenType::FemTet];

    eraseOldFunctor<FunctorType>(funcs, bodyId);

    auto functor = std::make_shared<FunctorType>();

    auto params = std::make_shared<PbdFemConstraintConfig>();
    params->setYoungAndPoisson(youngsModulus, poisson);

    functor->setBodyIndex(bodyId);
    functor->setFemConfig(params);
    functor->setMaterialType(material);

    funcs.push_back(std::move(functor));
}

void
PbdModelConfig::setBodyDamping(const int bodyId,
                               const double linearDampCoeff, const double angularDampCoeff)
{
    m_bodyLinearDampingCoeff[bodyId]  = linearDampCoeff;
    m_bodyAngularDampingCoeff[bodyId] = angularDampCoeff;
}

double
PbdModelConfig::getLinearDamping(const int bodyId)
{
    const double dampMult = (1.0 - m_linearDampingCoeff);
    auto         iter     = m_bodyLinearDampingCoeff.find(bodyId);
    if (iter != m_bodyLinearDampingCoeff.end())
    {
        const double bodyDampMult = (1.0 - iter->second);
        return 1.0 - (dampMult * bodyDampMult);
    }
    else
    {
        return m_linearDampingCoeff;
    }
}

double
PbdModelConfig::getAngularDamping(const int bodyId)
{
    const double dampMult = (1.0 - m_angularDampingCoeff);
    auto         iter     = m_bodyAngularDampingCoeff.find(bodyId);
    if (iter != m_bodyAngularDampingCoeff.end())
    {
        const double bodyDampMult = (1.0 - iter->second);
        return 1.0 - (dampMult * bodyDampMult);
    }
    else
    {
        return m_angularDampingCoeff;
    }
}
} // namespace imstk