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

namespace imstk {
class Scene : public Module
{
public:

    Scene(std::string name) : Module(name) {}

    ~Scene() = default;

    bool                                            isObjectRegistered(std::string sceneObjectName)
    const;
    const std::vector<std::shared_ptr<SceneObject> >getSceneObjects() const;
    std::shared_ptr<SceneObject>                    getSceneObject(std::string sceneObjectName)
    const;
    void                                            addSceneObject(
        std::shared_ptr<SceneObject>newSceneObject);
    void                                            removeSceneObject(std::string sceneObjectName);

protected:

    void initModule() override;
    void runModule() override;
    void cleanUpModule() override;

    std::unordered_map<std::string, std::shared_ptr<SceneObject> > m_sceneObjectsMap;
};
}

#endif // ifndef imstkScene_h
