/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"

#include "imstkPbdFemConstraint.h"

namespace imstk
{
class UnitCubeStrainScene : public ProgrammableScene
{
public:
    struct Configuration : ProgrammableScene::Configuration
    {
        Vec3i partitions;
        double strain;
        double compressionDuration;
        double youngsModulus;
        double poissonRatio;
        ProgrammableClient::DeformationType deformationType;
        PbdFemConstraint::MaterialType materialType;
    };

    UnitCubeStrainScene();
    UnitCubeStrainScene(const Configuration& cfg);

    bool hasAnalyticResults() override { return true; }
    std::string getName() const { return "UnitCubeStrain"; }
    bool setupScene(double sampleTime) override;
    bool setupView() override;
    bool trackData(double time) override;
    bool postProcessAnalyticResults() override;
    Configuration& getConfiguration() override { return m_config; }

protected:
    void setConfiguration(const Configuration& cfg);
    bool writeAnalyticStrainEnergyBaseline();

    Configuration m_config;
    std::shared_ptr<PbdObject> m_pbdCube;
};
} // namespace imstk
