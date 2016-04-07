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

#ifndef imstkScene_h
#define imstkScene_h

#include <vector>
#include <unordered_map>
#include <memory>

#include "imstkModule.h"
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"

namespace imstk {
class Scene : public Module
{
    template<class T>
    using NamedMap = std::unordered_map<std::string, std::shared_ptr<T>>;

public:

    Scene(std::string name) : Module(name) {}

    ~Scene() = default;

    bool isObjectRegistered(std::string sceneObjectName) const;
    const std::vector<std::shared_ptr<SceneObject>> getSceneObjects() const;
    std::shared_ptr<SceneObject> getSceneObject(std::string sceneObjectName) const;
    void addSceneObject(std::shared_ptr<SceneObject> newSceneObject);
    void removeSceneObject(std::string sceneObjectName);

    bool isLightRegistered(std::string lightName) const;
    const std::vector<std::shared_ptr<Light>> getLights() const;
    std::shared_ptr<Light> getLight(std::string lightName) const;
    void addLight(std::shared_ptr<Light> newLight);
    void removeLight(std::string lightName);

    std::shared_ptr<Camera> getCamera();

protected:

    void initModule() override;
    void runModule() override;
    void cleanUpModule() override;

    NamedMap<SceneObject>   m_sceneObjectsMap;
    NamedMap<Light>         m_lightsMap;
    std::shared_ptr<Camera> m_camera = std::make_shared<Camera>();
};
}

#endif // ifndef imstkScene_h
