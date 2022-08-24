/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkViewer.h"
#include "imstkCamera.h"
#include "imstkLogger.h"

namespace imstk
{
Viewer::Viewer(std::string name) :
    m_activeScene(nullptr),
    m_debugCamera(std::make_shared<Camera>()),
    m_screenCapturer(nullptr),
    m_config(std::make_shared<ViewerConfig>())
{
    // Set the preferred execution type
    m_executionType = ExecutionType::SEQUENTIAL;
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
Viewer::updateModule()
{
    this->postEvent(Event(Module::preUpdate()));
    this->postEvent(Event(Module::postUpdate()));
}
} // namespace imstk
