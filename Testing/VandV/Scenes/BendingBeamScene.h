/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "ProgrammableScene.h"
#include "imstkPbdObject.h"
// #include "imstkPbdStrainEnergyConstraint.h"

namespace imstk
{
class BendingBeamScene : public ProgrammableScene
{
public:
    struct Configuration : ProgrammableScene::Configuration
    {
        Vec3i partitions;
        double displacement;
        double youngsModulus;
        double poissonRatio;
        // PbdStrainEnergyConstraint::MaterialType materialType;

        void toString(std::ostream& str) const override;
    };

    BendingBeamScene();
    BendingBeamScene(const Configuration& cfg);

    bool hasAnalyticResults() override { return true; }
    std::string getName() const { return "BendingBeam"; }
    bool setupScene(double sampleTime) override;
    bool setupView() override;
    bool trackData(double time) override;
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
