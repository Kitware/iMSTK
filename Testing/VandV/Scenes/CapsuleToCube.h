/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "imstkPbdModelConfig.h"

namespace imstk
{
///
/// \class CapsuleToCubeScene
///
/// \brief Creates a rigid or deformabe cube and a capsule is moved to interact with (and optionally grasp) the cube
///
class CapsuleToCubeScene : public ProgrammableScene
{
public:
    /// Scene Type
    enum class Type { RigidCube = 0, DeformableCube, GraspDeformableCube };
    
    ///
    /// \struct Configuration
    ///
    /// \brief Configurable parameters for this scene
    ///
    struct Configuration : ProgrammableScene::Configuration
    {
        Type type;
        std::vector<PbdModelConfig::ConstraintGenType> constraintTypes;
        std::vector<double> constraintValues;
        Vec3d size      = Vec3d(1.0, 1.0, 1.0);
        Vec3i divisions =  Vec3i(3, 3, 3);
        double uniformMassValue = 0.0005;

        void toString(std::ostream& str) const override;
        void addConstraint(PbdModelConfig::ConstraintGenType& constraintType, double& constraintValue);
    };

    CapsuleToCubeScene();
    CapsuleToCubeScene(const Configuration& cfg);

    bool hasAnalyticResults() override { return false; }
    std::string getName() const { return "CapsuleToCube"; }

    ///
    /// \brief Sets the scene specific geometry
    /// \param How often to sample the scene data
    ///
    bool setupScene(double sampleTime) override;

    ///
    /// \brief Sets up a scene camera if the scene is rendering is enabled
    ///
    bool setupView() override;
    
    Configuration& getConfiguration() override { return m_config; }

protected:
    void setConfiguration(const Configuration& cfg);

    Configuration m_config;
};
} // namespace imstk
