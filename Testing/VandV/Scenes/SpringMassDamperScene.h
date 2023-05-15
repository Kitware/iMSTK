/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"

namespace imstk
{
class SpringMassDamperScene : public ProgrammableScene
{
public:
    struct Configuration : ProgrammableScene::Configuration
    {
        double initialLength;
        double elongationPercent;
        double mass;
        double stiffness;
    };

    SpringMassDamperScene();
    SpringMassDamperScene(const Configuration& cfg);

    bool hasAnalyticResults() override { return true; }
    std::string getName() const { return "SpringMassDamper"; }
    bool setupScene(double sampleTime) override;
    bool setupView() override;
    bool trackData(double time) override;
    bool postProcessAnalyticResults() override;
    Configuration& getConfiguration() override { return m_config; }

protected:
    void setConfiguration(const Configuration& cfg);
    bool writeAnalyticBaseline();

    Configuration m_config;
    std::shared_ptr<PbdObject> m_pbdSpring;

    DataTracker m_analyticTracker;
    std::vector<double> analyticPosition;
};
} // namespace imstk
