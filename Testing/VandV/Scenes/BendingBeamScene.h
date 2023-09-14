/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "imstkPbdObject.h"
#include "imstkPbdFemConstraint.h"

namespace imstk
{
///
/// \class BendingBeamScene
///
/// \brief Creates a simple weighted beam object to test stress and strains
///
class BendingBeamScene : public ProgrammableScene
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
        double displacement;
        double youngsModulus;
        double poissonRatio;
        PbdFemConstraint::MaterialType materialType;

        ///
        /// \brief Sets the provided string with a description the scene configuration used
        ///
        void toString(std::ostream& str) const override;
    };

    BendingBeamScene();
    BendingBeamScene(const Configuration& cfg);

    bool hasAnalyticResults() override { return true; }
    std::string getName() const { return "BendingBeam"; }

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
    /// \brief Scene specific post processing
    ///
    bool postProcessAnalyticResults() override;

    Configuration& getConfiguration() override { return m_config; }

protected:
    void setConfiguration(const Configuration& cfg);
    bool writeAnalyticBendingBeam();
    Configuration m_config;
    std::shared_ptr<PbdObject> m_pbdBeam;
    std::vector<int> m_trackedNodeIds;
    DataTracker      m_analyticTracker;

    CSVComparison m_analyticVerification;
    CSVComparison m_computedVerification;
    CSVComparison m_validation;
};
} // namespace imstk
