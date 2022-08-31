/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkFpsTxtCounter.h"
#include "imstkEntity.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkTextVisualModel.h"
#include "imstkViewer.h"

namespace imstk
{
FpsTxtCounter::FpsTxtCounter(const std::string& name) : Behaviour<double>(name),
    m_fpsTextVisualModel(std::make_shared<TextVisualModel>("FpsCounterTxt"))
{
    m_fpsTextVisualModel->setPosition(TextVisualModel::DisplayPosition::LowerLeft);
    m_fpsTextVisualModel->setFontSize(30.0);
}

void
FpsTxtCounter::init()
{
    // Add a visual representation for the object
    // how to avoid adding it twice?
    if (!m_entity->containsComponent(m_fpsTextVisualModel))
    {
        m_fpsTextVisualModel->setName(m_entity->getName() + "_FpsCounterTxt");
        m_entity->addComponent(m_fpsTextVisualModel);
    }
    CHECK(m_viewer.lock() != nullptr) << "FpsTxtCounter must have a Viewer to track";
}

void
FpsTxtCounter::visualUpdate(const double&)
{
    std::shared_ptr<Viewer> viewer    = m_viewer.lock();
    const int               infoLevel = viewer->getInfoLevel();
    if (infoLevel != m_prevInfoLevel)
    {
        if (infoLevel == 0)
        {
            m_fpsTextVisualModel->setVisibility(false);
        }
        else if (infoLevel == 1 || infoLevel == 2)
        {
            m_fpsTextVisualModel->setVisibility(true);
        }
    }
    m_prevInfoLevel = infoLevel;

    // Only update when visible
    if (m_fpsTextVisualModel->getVisibility())
    {
        std::shared_ptr<SceneManager> sceneManager = m_sceneManager.lock();

        std::string fpsVisualStr = "V: " +
                                   std::to_string(static_cast<int>(viewer->getVisualFps())) + " | " +
                                   "P: " +
                                   std::to_string(static_cast<int>(sceneManager->getActiveScene()->getFPS()));
        m_fpsTextVisualModel->setText(fpsVisualStr);
    }
}
} // namespace imstk