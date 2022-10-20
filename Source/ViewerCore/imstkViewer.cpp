/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkViewer.h"
#include "imstkAxesModel.h"
#include "imstkCamera.h"
#include "imstkEntity.h"
#include "imstkLogger.h"

namespace imstk
{
Viewer::Viewer(std::string name) :
    m_activeScene(nullptr),
    m_debugEntity(std::make_shared<Entity>("DebugEntity")),
    m_debugCamera(std::make_shared<Camera>()),
    m_screenCapturer(nullptr),
    m_config(std::make_shared<ViewerConfig>()),
    m_lastFpsUpdate(std::chrono::high_resolution_clock::now()),
    m_lastFps(60.0)
{
    // Set the preferred execution type
    m_executionType = ExecutionType::SEQUENTIAL;

    // Add a debug axes
    m_debugEntity->addComponent<AxesModel>();
}

void
Viewer::setDebugAxesLength(double x, double y, double z)
{
    m_debugEntity->getComponent<AxesModel>()->setScale(Vec3d(x, y, z));
}

std::shared_ptr<Renderer>
Viewer::getActiveRenderer() const
{
    CHECK(m_activeScene != nullptr) << "no active scene!";

    return m_rendererMap.at(m_activeScene);
}

void
Viewer::setInfoLevel(const int level)
{
    CHECK(level < getInfoLevelCount())
        << "There are only " << getInfoLevelCount() << " levels and level " << level << " was requested";
    m_infoLevel = level;
}

std::shared_ptr<KeyboardDeviceClient>
Viewer::getKeyboardDevice() const
{
    LOG(FATAL) << "No KeyboardDeviceClient implemented for Viewer";
    return nullptr;
}

std::shared_ptr<MouseDeviceClient>
Viewer::getMouseDevice() const
{
    LOG(FATAL) << "No MouseDeviceClient implemented for Viewer";
    return nullptr;
}

void
Viewer::updateFps()
{
    // Update framerate value display
    auto now = std::chrono::high_resolution_clock::now();
    m_visualFps = 1e6 / static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(now - m_pre).count());
    m_visualFps = 0.1 * m_visualFps + 0.9 * m_lastFps;
    m_lastFps   = m_visualFps;
    m_pre       = now;
}

void
Viewer::updateModule()
{
    this->postEvent(Event(Module::preUpdate()));
    this->postEvent(Event(Module::postUpdate()));
}
} // namespace imstk
