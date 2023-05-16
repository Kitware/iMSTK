/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "ProgrammableSceneExec.h"

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkVTKViewer.h"
#include "imstkKeyboardDeviceClient.h"

#include <regex>

namespace imstk
{
bool
ProgrammableSceneExec::executeScene(std::shared_ptr<ProgrammableScene> pScene)
{
    m_executedScenes.push_back(pScene);
    LOG(INFO) << "Setting up scene";
    pScene->setViewScene(m_viewScene);
    if (!pScene->setupScene(0.1))
    {
        return false;
    }

    double now      = 0;
    size_t numSteps = pScene->getNumSteps();

    if (!m_viewScene)
    {
        pScene->getScene()->initialize();
        for (size_t i = 0; i < numSteps; i++)
        {
            pScene->trackData(now);
            pScene->getScene()->advance(pScene->getTimeStep());

            for (auto pc : pScene->getClients())
            {
                pc->update();
            }

            now += pScene->getTimeStep();
        }
    }
    else
    {
        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        pScene->getScene()->addLight("Light", light);

        // Viewer
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(pScene->getScene());
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setDebugAxesLength(1.0, 1.0, 1.0);

        if (!pScene->setupView())
        {
            return false;
        }

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(pScene->getScene());

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(pScene->getTimeStep());

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        pScene->getScene()->addSceneObject(mouseAndKeyControls);

        connect<Event>(sceneManager, &SceneManager::preUpdate, [&](Event*)
            {
                size_t complete = 0;
                for (auto pc : pScene->getClients())
                {
                    pc->update();
                    complete += pc->isFinished();
                }
                if (complete >= pScene->getClients().size())
                {
                    driver->requestStatus(ModuleDriverStopped);
                }
                else
                {
                    pScene->trackData(now);
                    now += pScene->getTimeStep();
                }
                });

        connect<Event>(driver, &SimulationManager::ending, [&](Event*)
            {
                LOG(INFO) << "Completed scene";
                });

        driver->start();
    }

    return pScene->postProcessAnalysis();
}

void
reportError(CSVComparison& cmp, std::ostream& rpt)
{
    if (cmp.passed)
    {
        rpt << "<tr  bgcolor=\"#00FF00\">\n";
    }
    else
    {
        rpt << "<tr  bgcolor=\"#FF0000\">\n";
    }
    rpt << "  <td> " << cmp.name << "</td>\n";
    if (cmp.passed)
    {
        rpt << "  <td> 0 </td>\n";
    }
    else
    {
        rpt << "  <td>\n";
        size_t totalErrors = 0;
        for (auto& itr : cmp.computedSummary)
        {
            totalErrors += itr.second.numErrors;
        }
        rpt << totalErrors << " total errors found<br>\n";
        for (auto& itr : cmp.computedSummary)
        {
            if (itr.second.numErrors)
            {
                rpt << itr.first << " has " << itr.second.numErrors << " [" << itr.second.minError << ", " << itr.second.maxError << "]" << " from timstep " << itr.second.errorStart <<
                    " to timestep " << itr.second.errorEnd << "<br>\n";
            }
            else
            {
                rpt << itr.first << " has " << itr.second.numErrors << " [" << itr.second.minError << ", " << itr.second.maxError << "]<br>\n";
            }
        }
        rpt << "</td>\n";
    }
    rpt << "</tr>\n";
}

void
ProgrammableSceneExec::report()
{
    std::string   s;
    std::ofstream rpt;
    rpt.open("./test_results/VandVreport.html");
    rpt << "<!DOCTYPE html><html><title>iMSTk VandV Run</title><body>\n";

    for (auto scene : m_executedScenes)
    {
        rpt << "<h2>" + scene->getName() + "</h2>\n";
        s = scene->getConfiguration().toString();
        rpt << std::regex_replace(s, std::regex("\n"), "<br>") << "\n";
        rpt << "<br>\n";

        auto& solver = scene->m_solverComparison.computedSummary;
        rpt << "<table border=\"1\">\n";
        rpt << "<caption><b> Solver Stats </b></caption>\n";
        rpt << "<tr>\n";
        rpt << "  <th> Total Solver Time (ms) </th>\n";
        rpt << "  <th> Mean Solver Time (ms)</th>\n";
        rpt << "  <th> Mean # Constraints </th>\n";
        rpt << "  <th> Mean Constraint (ms)</th>\n";
        rpt << "</tr>\n";
        rpt << "<tr>\n";
        rpt << "  <td> " << solver[DataTracker::Physics::SolverTime_ms].sum << "</td>\n";
        rpt << "  <td> " << solver[DataTracker::Physics::SolverTime_ms].mean << "</td>\n";
        rpt << "  <td> " << solver[DataTracker::Physics::NumConstraints].mean << "</td>\n";
        rpt << "  <td> " << solver[DataTracker::Physics::AverageC].mean << "</td>\n";
        rpt << "</tr>\n";
        rpt << "</table><br>\n";

        rpt << "<table border=\"1\">\n";
        rpt << "<caption><b> Verification </b></caption>\n";
        rpt << "<tr>\n";
        rpt << "  <th> Test Case </th>\n";
        rpt << "  <th> Failures </th>\n";
        rpt << "</tr>\n";
        rpt << "<tr>\n";
        reportError(scene->m_solverComparison, rpt);
        for (int i = 0; i < scene->m_positionAndVelocityComparision.size(); i++)
        {
            reportError(scene->m_positionAndVelocityComparision[i], rpt);
            reportError(scene->m_displacementComparision[i], rpt);
        }
        rpt << "</table><br>\n";
    }

    rpt << "</body></html>\n";
    rpt.close();
}
} // end namespace imstk
