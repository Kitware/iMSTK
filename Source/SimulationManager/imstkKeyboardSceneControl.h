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

#pragma once

#include "imstkKeyboardControl.h"

namespace imstk
{
class SceneManager;
class Viewer;

///
/// \class KeyboardSceneControl
///
/// \brief This implements keyboard controls to control a scene manager and viewer
/// it may operate with both sceneManager and viewer, or only one or the other.
/// warning: This control may pause a thread. Thus it is a bad idea to process it
/// on the thread you're pausing, as you then could not resume.
/// \todo: Thread affinity
///
class KeyboardSceneControl : public KeyboardControl
{
public:
    KeyboardSceneControl();
    KeyboardSceneControl(std::shared_ptr<KeyboardDeviceClient> keyDevice);
    virtual ~KeyboardSceneControl() override = default;

public:
    void setSceneManager(std::shared_ptr<SceneManager> sceneManager) { m_sceneManager = sceneManager; }
    std::shared_ptr<SceneManager> getSceneManager() const { return m_sceneManager; }

    void setViewer(std::shared_ptr<Viewer> viewer) { m_viewer = viewer; }
    std::shared_ptr<Viewer> getViewer() const { return m_viewer; }

public:
    void printControls() override;

    void OnKeyPress(const char key) override;

    void OnKeyRelease(const char key) override;

protected:
    std::shared_ptr<SceneManager> m_sceneManager;
    std::shared_ptr<Viewer>       m_viewer;
    bool m_showFps;
};
}