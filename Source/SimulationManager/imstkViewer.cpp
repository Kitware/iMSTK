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
#include "imstkInteractorStyle.h"
#include "imstkLogger.h"

#ifdef iMSTK_USE_Vulkan
#include "imstkGUICanvas.h"
#endif

namespace imstk
{
Viewer::Viewer(std::string name) :
    m_activeScene(nullptr),
    m_config(std::make_shared<ViewerConfig>()),
    m_debugCamera(std::make_shared<Camera>()),
    m_interactorStyle(nullptr),
    m_screenCapturer(nullptr)
{
#ifdef iMSTK_USE_Vulkan
    m_canvas(std::make_shared<GUIOverlay::Canvas>())
#endif
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
Viewer::updateModule()
{
    this->postEvent(Event(EventType::PreUpdate));
    for (auto control : m_controls)
    {
        control->update(m_dt);
    }
    this->postEvent(Event(EventType::PostUpdate));
}
}
