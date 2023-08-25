/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "imstkDataTracker.h"
#include "imstkScene.h"
#include "imstkPbdModel.h"
#include "imstkProgrammableClient.h"
#include "Verification/CSVUtils.h"
#include "Verification/EntityToVTK.h"

namespace imstk
{
class ProgrammableScene
{
public:
    struct Configuration
    {
        double dt = 0.001;
        unsigned int iterations = 1;
        std::string rootOutputDirectory  = "./test_results/";
        std::string sceneOutputDirectory = "";

        std::string toString() const;
        virtual void toString(std::ostream& str) const;
    };
protected:
    ProgrammableScene();
    void setConfiguration(const Configuration&);
public:
    virtual ~ProgrammableScene() = default;

    virtual std::string getName() const = 0;

    virtual bool setupScene(double sampleTime);
    virtual void setViewScene(bool viewScene) { m_viewScene = viewScene; }
    virtual bool setupView() { return true; };
    virtual bool trackData(double time);
    virtual bool postProcessAnalysis();
    virtual bool hasAnalyticResults() = 0;
    virtual bool postProcessAnalyticResults() { return true; }
    virtual Configuration& getConfiguration() = 0;

    double getTimeStep() const;
    double getDuration() const { return m_duration; }
    size_t getNumSteps() const { return m_numSteps; }
    std::shared_ptr<Scene> getScene() { return m_scene; }
    std::vector<std::shared_ptr<ProgrammableClient>>& getClients() { return m_clients; }

    std::string m_outDir;
    DataTracker m_dataTracker;
    double      m_duration = 0;
    size_t      m_numSteps = 0;
    std::shared_ptr<Scene>    m_scene;
    std::shared_ptr<PbdModel> m_pbdModel;
    std::vector<std::shared_ptr<ProgrammableClient>> m_clients;
    std::shared_ptr<EntityToVTK> m_entity2vtk;
    CSVComparison m_solverComparison;
    CSVComparison m_analyticVerification; // analytic baseline vs computed analytic
    CSVComparison m_computedVerification; // scene computed baseline vs scene computed
    CSVComparison m_analyticValidation;   // computed analytic vs. scene computed
    CSVComparison m_recordingDisplacement;
    CSVComparison m_vtkDisplacement;
    std::vector<CSVComparison> m_displacementComparision;
    std::vector<CSVComparison> m_positionAndVelocityComparision;
    bool m_viewScene = false;
};
inline std::ostream&
operator<<(std::ostream& out, const ProgrammableScene::Configuration& cfg)
{
    cfg.toString(out);
    return out;
}
} // namespace imstk
