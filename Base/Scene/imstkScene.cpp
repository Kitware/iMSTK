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

#include "imstkScene.h"

#include <g3log/g3log.hpp>

namespace imstk {
bool
Scene::isObjectRegistered(std::string sceneObjectName) const
{
    return m_sceneObjectsMap.find(sceneObjectName) != m_sceneObjectsMap.end();
}

const std::vector<std::shared_ptr<SceneObject> >
Scene::getSceneObjects() const
{
    std::vector<std::shared_ptr<SceneObject> > v;

    for (auto it = m_sceneObjectsMap.begin();
         it != m_sceneObjectsMap.end();
         ++it)
    {
        v.push_back(it->second);
    }

    return v;
}

std::shared_ptr<SceneObject>
Scene::getSceneObject(std::string sceneObjectName) const
{
    if (!this->isObjectRegistered(sceneObjectName))
    {
        LOG(WARNING) << "No scene object named '" << sceneObjectName
                     << "' was registered in this scene.";
        return nullptr;
    }

    return m_sceneObjectsMap.at(sceneObjectName);
}

void
Scene::addSceneObject(std::shared_ptr<SceneObject>newSceneObject)
{
    std::string newSceneObjectName = newSceneObject->getName();

    if (this->isObjectRegistered(newSceneObjectName))
    {
        LOG(WARNING) << "Can not add object: '" << newSceneObjectName
                     << "' is already registered in this scene.";
        return;
    }

    m_sceneObjectsMap[newSceneObjectName] = newSceneObject;
    LOG(INFO) << newSceneObjectName << " object added to " << m_name;
}

void
Scene::removeSceneObject(std::string sceneObjectName)
{
    if (!this->isObjectRegistered(sceneObjectName))
    {
        LOG(WARNING) << "No object named '" << sceneObjectName
                     << "' was registered in this scene.";
        return;
    }

    m_sceneObjectsMap.erase(sceneObjectName);
    LOG(INFO) << sceneObjectName << " object removed from " << m_name;
}

bool
Scene::isLightRegistered(std::string lightName) const
{
    return m_lightsMap.find(lightName) != m_lightsMap.end();
}

const std::vector<std::shared_ptr<Light>>
Scene::getLights() const
{
    std::vector<std::shared_ptr<Light>> v;

    for (auto it = m_lightsMap.begin();
         it != m_lightsMap.end();
         ++it)
    {
        v.push_back(it->second);
    }

    return v;
}

std::shared_ptr<Light>
Scene::getLight(std::string lightName) const
{
    if (!this->isLightRegistered(lightName))
    {
        LOG(WARNING) << "No scene light named '" << lightName
                     << "' was registered in this scene.";
        return nullptr;
    }

    return m_lightsMap.at(lightName);
}

void
Scene::addLight(std::shared_ptr<Light>newLight)
{
    std::string newlightName = newLight->getName();

    if (this->isLightRegistered(newlightName))
    {
        LOG(WARNING) << "Can not add light: '" << newlightName
                     << "' is already registered in this scene.";
        return;
    }

    m_lightsMap[newlightName] = newLight;
    LOG(INFO) << newlightName << " light added to " << m_name;
}

void
Scene::removeLight(std::string lightName)
{
    if (!this->isLightRegistered(lightName))
    {
        LOG(WARNING) << "No light named '" << lightName
                     << "' was registered in this scene.";
        return;
    }

    m_lightsMap.erase(lightName);
    LOG(INFO) << lightName << " light removed from " << m_name;
}

void
Scene::initModule()
{
    LOG(DEBUG) << m_name << " : init";
}

void
Scene::cleanUpModule()
{
    LOG(DEBUG) << m_name << " : cleanUp";
}

void
Scene::runModule()
{
    LOG(DEBUG) << m_name << " : running";
}
}
