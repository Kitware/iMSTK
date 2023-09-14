/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "imstkPbdModelConfig.h"

namespace imstk
{
///
/// \class LoadableMeshScene
///
/// \brief Loads a mesh and creates a capsule to move and interact with it
///
class LoadableMeshScene : public ProgrammableScene
{
public:
    ///
    /// \struct Configuration
    ///
    /// \brief Configurable parameters for this scene
    ///
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
