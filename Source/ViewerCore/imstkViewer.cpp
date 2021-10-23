/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkViewer.h"
#include "imstkCamera.h"
#include "imstkDeviceControl.h"
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
Viewer::setInfoLevel(int level)
{
    CHECK(level < getInfoLevelCount())
        << "There are only " << getInfoLevelCount() << " levels and level " << level << " was requested";
    m_infoLevel = level;
}

void
Viewer::updateModule()
{
    this->postEvent(Event(Module::preUpdate()));
    for (auto control : m_controls)
    {
        control->update(m_dt);
    }
    this->postEvent(Event(Module::postUpdate()));
}
}
