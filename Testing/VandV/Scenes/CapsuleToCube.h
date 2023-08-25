/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "imstkPbdModelConfig.h"

namespace imstk
{
class CapsuleToCubeScene : public ProgrammableScene
{
public:
    enum class Type { RigidCube = 0, DeformableCube, GraspDeformableCube };
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
    bool setupScene(double sampleTime) override;
    bool setupView() override;
    Configuration& getConfiguration() override { return m_config; }

protected:
    void setConfiguration(const Configuration& cfg);

    Configuration m_config;
};
} // namespace imstk
