/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "ProgrammableScene.h"
#include "Verification/CSVUtils.h"

#include "imstkPbdModelConfig.h"

#include "vtksys/SystemTools.hxx"

namespace imstk
{
std::string
ProgrammableScene::Configuration::toString() const
{
    std::stringstream str;
    toString(str);
    return str.str();
}

void
ProgrammableScene::Configuration::toString(std::ostream& str) const
{
    str << "\tOutputDirectory: " << rootOutputDirectory << sceneOutputDirectory << "\n";
    str << "\tdt: " << dt << "\n";
    str << "\titerations: " << iterations << "\n";
}

ProgrammableScene::ProgrammableScene()
{
    m_solverComparison.name     = "Solver Comparison";
    m_analyticVerification.name = "Analytic Verification";
    m_computedVerification.name = "Computed Verification";
    m_analyticValidation.name   = "Analytic Verification";
}

void
ProgrammableScene::setConfiguration(const Configuration& cfg)
{
    m_outDir = cfg.rootOutputDirectory;
    if (cfg.sceneOutputDirectory.empty())
    {
        m_outDir += getName() + "/";
    }
    else
    {
        m_outDir += cfg.sceneOutputDirectory;
    }
    LOG(INFO) << "Setting up scene directory: " << m_outDir;
    vtksys::SystemTools::RemoveADirectory(m_outDir);
    vtksys::SystemTools::MakeDirectory(m_outDir);

    m_scene    = std::make_shared<Scene>("Scene");
    m_pbdModel = std::make_shared<PbdModel>();
    auto pbdCfg = m_pbdModel->getConfig();
    pbdCfg->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdCfg->m_dt         = cfg.dt;
    pbdCfg->m_iterations = cfg.iterations;
    pbdCfg->m_linearDampingCoeff  = 0.0;
    pbdCfg->m_angularDampingCoeff = 0.0;
    pbdCfg->m_doPartitioning      = false;
    pbdCfg->m_dataTracker = std::make_shared<DataTracker>();
    pbdCfg->m_dataTracker->setFilename(m_outDir + "solver.csv");
    m_entity2vtk = std::make_shared<EntityToVTK>();

    vtksys::SystemTools::MakeDirectory(m_outDir + "paraview_series/");
    m_entity2vtk->setTimeSeriesFilePath(m_outDir + "paraview_series/");
    m_dataTracker.setFilename(m_outDir + "computed.csv");
}

double
ProgrammableScene::getTimeStep() const
{
    return m_pbdModel->getConfig()->m_dt;
}

bool
ProgrammableScene::setupScene(double sampleTime)
{
    // EntityToVTK breaks if scene is not visible
    // if (!m_viewScene)
    // {
        for (auto obj :m_scene->getSceneObjects())
        {
            auto pbdObject = std::dynamic_pointer_cast<PbdObject>(obj);
            if (pbdObject)
            {
                m_entity2vtk->addEntity(obj);
                CSVComparison displacementCompare;
                displacementCompare.name = obj->getName() + "_timestep_displacement";
                m_displacementComparision.emplace_back(displacementCompare);

                CSVComparison posAndVelCompare;
                posAndVelCompare.name = obj->getName() + "_position_and_velocity";
                m_positionAndVelocityComparision.emplace_back(posAndVelCompare);
                
            }
        }
    // }
    m_entity2vtk->setTimeBetweenRecordings(sampleTime);
    m_numSteps = m_duration / m_pbdModel->getConfig()->m_dt;
    return true;
}

bool
ProgrammableScene::trackData(double time)
{
    m_entity2vtk->recordObjectState(getTimeStep());
    m_pbdModel->getConfig()->m_dataTracker->streamProbesToFile(time);
    return true;
}

bool
ProgrammableScene::postProcessAnalysis()
{
    LOG(INFO) << "Starting analysis and processing of results";
    m_entity2vtk->writeTimeSeriesJsonFile();
    m_entity2vtk->writeObjectsToFile(m_outDir + "paraview.vtm");
    for (int i = 0; i < m_positionAndVelocityComparision.size(); i++)
    {
        auto objName = m_positionAndVelocityComparision[i].name.substr(0, m_positionAndVelocityComparision[i].name.find("_"));
        m_entity2vtk->compareMultiBlock(m_outDir + "paraview.vtm", objName, m_outDir + m_displacementComparision[i].name + ".csv");
        m_entity2vtk->compareMultiBlocks(m_outDir + "paraview.vtm", m_outDir + "paraview.vtm", objName, m_outDir + m_positionAndVelocityComparision[i].name + ".csv");
    }
    // Create a csv from comparing a baseline vtk dir and our computed vtk dir
    //createVTKCSVs();
    //compareAndPlotCSVs(m_outDir + "vtk_baseline.csv", m_outDir + "vtk_computed.csv", m_outDir + "vtk_results/");
    // Analysis and reporting phase
    if (hasAnalyticResults())
    {
        postProcessAnalyticResults();
    }

    // We don't have constraint baselines yet, so just compare with ourselves
    compareAndPlotTimeSeriesCSVs(m_outDir + "solver.csv", m_outDir + "solver.csv", m_solverComparison, m_outDir + "solver_results/", false);
    if (m_entity2vtk->getRecordingType() != EntityToVTK::RecordingType::NoRecording)
    {
        for (int i = 0; i < m_positionAndVelocityComparision.size(); i++)
        {
            compareAndPlotTimeSeriesCSVs(m_outDir + m_positionAndVelocityComparision[i].name + ".csv", m_outDir + m_positionAndVelocityComparision[i].name + ".csv",
                                         m_positionAndVelocityComparision[i],
                                        m_outDir + m_positionAndVelocityComparision[i].name + "_results/", false);
            compareAndPlotTimeSeriesCSVs(m_outDir + m_displacementComparision[i].name + ".csv", m_outDir + m_displacementComparision[i].name + ".csv", m_displacementComparision[i],
                                        m_outDir + m_displacementComparision[i].name + "_results/", false);
        }
    }

    return true;
}
} // namespace imstk
