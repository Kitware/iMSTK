/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkKeyboardSceneControl.h"
#include "imstkCamera.h"
#include "imstkLogger.h"
#include "imstkModuleDriver.h"
#include "imstkRenderer.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkViewer.h"
#include "imstkVisualModel.h"
#include "imstkTextVisualModel.h"

namespace imstk
{
KeyboardSceneControl::KeyboardSceneControl(const std::string& name) : KeyboardControl(name)
{
    // Create visual model
    m_textVisualModel = std::make_shared<TextVisualModel>();
    m_textVisualModel->setFontSize(40);
    m_textVisualModel->setVisibility(false);
    m_textVisualModel->setText(
        "Simulation Paused\nPress Space to Continue\nPress R to Reset\nPress C to clear pause screen");

    m_textVisualModel->setPosition(TextVisualModel::DisplayPosition::CenterCenter);

    addVisualModel(m_textVisualModel);
}

bool
KeyboardSceneControl::initialize()
{
    m_textVisualModel->setVisibility(m_useTextStatus ? m_sceneManager.lock()->getPaused() : false);
    return true;
}

void
KeyboardSceneControl::printControls()
{
    LOG(INFO) << "Keyboard Scene Controls:";
    LOG(INFO) << "----------------------------------------------------------------------";
    LOG(INFO) << " | Space - pause/resume the scene";
    LOG(INFO) << " | q/Q   - stop both the scene and viewer";
    LOG(INFO) << " | d/D   - toggle debug/simulation mode";
    LOG(INFO) << " | p/P   - toggle visual/physics updates/second and task times display";
    LOG(INFO) << " | r/R   - reset the simulation";
    LOG(INFO) << " | n/N   - print camera pose of the active scene's camera";
    LOG(INFO) << "----------------------------------------------------------------------";
}

void
KeyboardSceneControl::OnKeyPress(const char key)
{
    auto sceneManager = m_sceneManager.lock();

    if (sceneManager == nullptr)
    {
        LOG(WARNING) << "Keyboard control disabled: No scene manager provided";
        return;
    }
    auto driver = m_driver.lock();
    if (driver == nullptr)
    {
        LOG(WARNING) << "Keyboard control disabled: No driver provided";
        return;
    }

    // Pause/Resume the simulation
    if (key == ' ')
    {
        // To ensure consistency toggle/invert based of m_sceneManager
        const bool paused = sceneManager->getPaused();

        // Switch pause screen visibility
        m_textVisualModel->setVisibility(m_useTextStatus ? !paused : false);

        // Resume or pause all modules, expect viewers
        for (auto module : driver->getModules())
        {
            if (std::dynamic_pointer_cast<Viewer>(module) == nullptr)
            {
                module->setPaused(!paused);
            }
        }
        // In case the SceneManager is not apart of the driver
        paused ? sceneManager->resume() : sceneManager->pause();
    }
    // Stop the program
    else if (key == 'q' || key == 'Q' || key == 'e' || key == 'E') // end Simulation
    {
        driver->requestStatus(ModuleDriverStopped);
    }
    // Switch to debug mode
    else if (key == 'd' || key == 'D') // switch rendering mode of the modules
    {
        // To ensure consistency toggle/invert based of m_sceneManager
        const bool simModeOn = sceneManager->getMode() == SceneManager::Mode::Simulation ? true : false;

        for (auto module : driver->getModules())
        {
            if (auto subManager = std::dynamic_pointer_cast<SceneManager>(module))
            {
                if (simModeOn)
                {
                    subManager->setMode(SceneManager::Mode::Debug);
                }
                else
                {
                    subManager->setMode(SceneManager::Mode::Simulation);
                }
            }
            if (auto viewer = std::dynamic_pointer_cast<Viewer>(module))
            {
                if (simModeOn)
                {
                    viewer->setRenderingMode(Renderer::Mode::Debug);
                }
                else
                {
                    viewer->setRenderingMode(Renderer::Mode::Simulation);
                }
            }
        }

        simModeOn ? sceneManager->setMode(SceneManager::Mode::Debug) : sceneManager->setMode(SceneManager::Mode::Simulation);
    }
    // Toggle through info levels
    else if (key == 'p' || key == 'P')
    {
        for (auto module : driver->getModules())
        {
            std::shared_ptr<Viewer> viewer = std::dynamic_pointer_cast<Viewer>(module);
            if (viewer != nullptr)
            {
                viewer->setInfoLevel((viewer->getInfoLevel() + 1) % viewer->getInfoLevelCount());
            }
        }
    }
    // Reset simulation
    else if (key == 'r' || key == 'R')
    {
        sceneManager->getActiveScene()->reset();
    }
    else if (key == 'n' || key == 'N')
    {
        sceneManager->getActiveScene()->getActiveCamera()->print();
    }
    // Toggle text on pause screen
    else if (key == 'c' || key == 'C')
    {
        m_useTextStatus = !m_useTextStatus;
        const bool paused = sceneManager->getPaused();
        m_textVisualModel->setVisibility(m_useTextStatus ? !paused : false);
    }
}

void
KeyboardSceneControl::OnKeyRelease(const char imstkNotUsed(key))
{
}
} // namespace imstk