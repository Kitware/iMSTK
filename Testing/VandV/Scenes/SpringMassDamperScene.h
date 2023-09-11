/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"

namespace imstk
{
///
/// \class SpringMassDamperScene
///
/// \brief Creates a simple spring mass damper and validates against an analytic implementation
///
class SpringMassDamperScene : public ProgrammableScene
{
public:
    ///
    /// \struct Configuration
    ///
    /// \brief Configurable parameters for this scene
    ///
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

    ///
    /// \brief Sets the scene specific geometry
    /// \param How often to sample the scene data
    ///
    bool setupScene(double sampleTime) override;

    ///
    /// \brief Sets up a scene camera if the scene is rendering is enabled
    ///
    bool setupView() override;

    ///
    /// \brief Record scene specific data to the data track file for the provided sim time
    ///
    bool trackData(double time) override;

    ///
    /// \brief Scene specific post processing for analytic implementations
    ///
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
