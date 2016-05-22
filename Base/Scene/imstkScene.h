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

#include <unordered_map>
#include <memory>

#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkCollisionGraph.h"

namespace imstk
{

class Scene
{
    template<class T>
    using NamedMap = std::unordered_map<std::string, std::shared_ptr<T>>;

public:

    Scene(std::string name) : m_name(name) {}

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

    const std::string& getName() const;

    std::shared_ptr<Camera> getCamera() const;
    std::shared_ptr<CollisionGraph> getCollisionGraph() const;

protected:

    std::string m_name;
    NamedMap<SceneObject> m_sceneObjectsMap;
    NamedMap<Light> m_lightsMap;
    std::shared_ptr<Camera> m_camera = std::make_shared<Camera>();
    std::shared_ptr<CollisionGraph> m_collisionGraph = std::make_shared<CollisionGraph>();
};
}

#endif // ifndef imstkScene_h
