/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPerformanceGraph.h"
#include "imstkRenderer.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkViewer.h"

namespace imstk
{
void
PerformanceGraph::visualUpdate(const double& dt)
{
    std::shared_ptr<Viewer> viewer    = m_viewer.lock();
    const int               infoLevel = viewer->getInfoLevel();
    if (infoLevel != m_prevInfoLevel)
    {
        // If level 1 or 2 enable, else disable
        m_enabled = (infoLevel == 2);
        m_sceneManager.lock()->getActiveScene()->setEnableTaskTiming(m_enabled);
        viewer->getActiveRenderer()->setTimeTableVisibility(m_enabled);
    }
    m_prevInfoLevel = infoLevel;

    // Only update when visible
    if (m_enabled)
    {
        m_t += dt;

        if (m_t > m_updateDuration) // wait 150ms before updating displayed value
        {
            std::shared_ptr<SceneManager> sceneManager = m_sceneManager.lock();
            std::shared_ptr<Scene>        scene = sceneManager->getActiveScene();
            std::shared_ptr<Renderer>     ren   = viewer->getActiveRenderer();

            // Update the timing table
            scene->lockComputeTimes();
            ren->setTimeTable(scene->getTaskComputeTimes());
            scene->unlockComputeTimes();
            m_t = 0.0;
        }
    }
}
} // namespace imstk