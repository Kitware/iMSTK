/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "imstkPbdModelConfig.h"

namespace imstk
{
class LoadableMeshScene : public ProgrammableScene
{
public:
    struct Configuration : ProgrammableScene::Configuration
    {
        std::string filePath;
        double mass;
        std::vector<PbdModelConfig::ConstraintGenType> constraintTypes;
        std::vector<double> constraintValues;
    };

    LoadableMeshScene();
    LoadableMeshScene(const Configuration& cfg);

    bool hasAnalyticResults() override { return false; }
    std::string getName() const { return "LoadableMesh"; }
    bool setupScene(double sampleTime) override;
    bool setupView() override;
    Configuration& getConfiguration() override { return m_config; }

protected:
    void setConfiguration(const Configuration& cfg);

    Configuration m_config;
};
} // namespace imstk
