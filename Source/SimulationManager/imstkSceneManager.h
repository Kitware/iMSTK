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

#include <unordered_map>
#include <memory>
#include <thread>

#include "imstkModule.h"
#include "imstkScene.h"

namespace imstk
{
///
/// \class SceneManager
///
/// \brief
///
class SceneManager : public Module
{
public:
    ///
    /// \brief Constructor
    ///
    SceneManager(std::shared_ptr<Scene> scene) :
        Module(scene->getName()),
        m_scene(scene) {}

    ///
    /// \brief Destructor
    ///
    virtual ~SceneManager() override = default;

    ///
    /// \brief Get the scene that the scene manager is managing
    ///
    std::shared_ptr<Scene> getScene() { return m_scene; };

protected:
    ///
    /// \brief Initialize the module
    ///
    void initModule() override;

    ///
    /// \brief Run the module
    ///
    void runModule() override { m_scene->advance(); };

    ///
    /// \brief Clean up the module
    ///
    void cleanUpModule() override;

    ///
    /// \brief
    ///
    void startModuleInNewThread(std::shared_ptr<Module> module);

    std::shared_ptr<Scene> m_scene;                           ///> Scene that is being managed
    std::unordered_map<std::string, std::thread> m_threadMap; ///>        
};
} // imstk
