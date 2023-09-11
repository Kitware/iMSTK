/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"

#include "imstkPbdFemConstraint.h"

namespace imstk
{
///
/// \class UnitCubeStrainScene
///
/// \brief Creates a simple cube to test and validates stress and strain against an analytic implementation
///
class UnitCubeStrainScene : public ProgrammableScene
{
public:
    ///
    /// \struct Configuration
    ///
    /// \brief Configurable parameters for this scene
    ///
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
    bool writeAnalyticStrainEnergyBaseline();

    Configuration m_config;
    std::shared_ptr<PbdObject> m_pbdCube;
};
} // namespace imstk
