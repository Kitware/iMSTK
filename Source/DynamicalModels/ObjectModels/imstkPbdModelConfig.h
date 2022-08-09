/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdCollisionConstraint.h"
#include "imstkPbdFemConstraint.h"
#include "imstkPbdConstraintFunctor.h"

#include <unordered_map>
#include <unordered_set>

namespace imstk
{
///
/// \class PbdModelConfig
///
/// \brief Gives parameters for PBD simulation
///
class PbdModelConfig
{
public:
    ///
    /// \brief Gives the set of standard pbd constraint generation schemes/functors
    /// provided by iMSTK. Note, these do not correspond to constraint types
    /// as there may be multiple schemes for one constraint or even multiple
    /// constraints per scheme
    ///
    enum class ConstraintGenType
    {
        Custom,
        Distance,
        FemTet,
        Volume,
        Area,
        Bend,
        Dihedral,
        ConstantDensity
    };

public:
    ///
    /// \brief Enables a constraint of type defined by ConstraintGenType with
    /// given stiffness. If constraint of that type already exists, sets the
    /// stiffness on it.
    /// Defaults to bodyId=1, the first body, where 0 is the dummy body
    ///
    void enableConstraint(ConstraintGenType type, const double stiffness, const int bodyId = 2);

    ///
    /// \brief Enables a bend constraint with given stiffness, stride, and flag for 0 rest length
    /// You may enable multiple with differing strides
    /// If constraint with same stride already exists, updates the stiffness and restLength0 on it
    /// \param Stiffness, how much bend is enforced
    /// \param Stride, distance between vertex connections
    /// \param When true rest length (and angle) are constrained to 0, useful when mesh initial/resting state
    /// is not 0 angled
    /// \param Body to add the constraint throughout
    ///
    void enableBendConstraint(const double stiffness, const int stride, const bool restLength0 = true, const int bodyId = 2);

    ///
    /// \brief Enables constant density constraint given the stiffness and particleSize
    /// \param Stiffness, how much density is enforced
    /// \param ParticleRadius, radius of particle
    /// \param Body to add the constraint throughout
    ///
    void enableConstantDensityConstraint(const double stiffness,
                                         const double particleRadius, const double restDensity = 6378.0, const int bodyId = 2);

    ///
    /// \brief Enable a Fem constraint with the material provided
    ///
    void enableFemConstraint(PbdFemConstraint::MaterialType material, const int bodyId = 2);

    ///
    /// \brief If lame parameters (mu+lambda) are given in femParams, then youngs modulus and poissons ratio are computed
    /// Conversly if youngs and poissons are given, lame parameters are computed
    ///
    void computeElasticConstants();

    ///
    /// \brief Adds a functor to generate constraints
    /// \tparam Must contain operator(PbdConstraintContainer&), could be a PbdConstraintFunctor
    /// or std::function<void(PbdConstraintContainer&)>
    void addPbdConstraintFunctor(std::shared_ptr<PbdConstraintFunctor> functor)
    {
        m_functors[ConstraintGenType::Custom].push_back(functor);
    }

    void addPbdConstraintFunctor(std::function<void(PbdConstraintContainer&)> functor)
    {
        m_functors[ConstraintGenType::Custom].push_back(
            std::make_shared<PbdConstraintFunctorLambda>(functor));
    }

    std::unordered_map<ConstraintGenType, std::vector<std::shared_ptr<PbdConstraintFunctor>>>& getFunctors() { return m_functors; }

    ///
    /// \brief Set damping for a specific body
    /// 1.0 is fully damped/all velocity removed, 0.0 is no damping
    ///
    void setBodyDamping(const int bodyId,
                        const double linearDampCoeff, const double angularDampCoeff = 0.01);

    ///
    /// \brief Returns global and per body damping multiplied together
    /// for a body
    /// 1.0 is fully damped/all velocity removed, 0.0 is no damping
    ///@{
    double getLinearDamping(const int bodyId);
    double getAngularDamping(const int bodyId);
///@}

public:
    double m_linearDampingCoeff  = 0.01;      ///< Damping coefficient applied to linear velocity [0, 1]
    double m_angularDampingCoeff = 0.01;      ///< Damping coefficient applied to angular velcoity [0, 1]

    unsigned int m_iterations = 10;           ///< Internal constraints pbd solver iterations
    unsigned int m_collisionIterations = 5;
    double       m_dt     = 0.01;             ///< Time step size
    bool m_doPartitioning = true;             ///< Does graph coloring to solve in parallel

    Vec3d m_gravity = Vec3d(0.0, -9.81, 0.0); ///< Gravity acceleration

    std::shared_ptr<PbdFemConstraintConfig> m_femParams =
        std::make_shared<PbdFemConstraintConfig>(PbdFemConstraintConfig
        {
            0.0,                // Lame constant, if constraint type is FEM
            0.0,                // Lame constant, if constraint type is FEM
            1000.0,             // FEM parameter, if constraint type is FEM
            0.2                 // FEM parameter, if constraint type is FEM
            });

    PbdConstraint::SolverType m_solverType = PbdConstraint::SolverType::xPBD;

    std::unordered_map<int, double> m_bodyLinearDampingCoeff;  ///< Per body linear damping, Body id -> linear damping for given body [0, 1]
    std::unordered_map<int, double> m_bodyAngularDampingCoeff; ///< Per body angular damping, Body id -> angular damping for given body [0, 1]

protected:
    friend class PbdModel;

    std::unordered_map<ConstraintGenType, std::vector<std::shared_ptr<PbdConstraintFunctor>>> m_functors;
};
} // namespace imstk