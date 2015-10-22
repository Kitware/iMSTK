// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Core/Scene.h"


Scene::Scene(std::shared_ptr<ErrorLog> p_log) :
    CoreClass()
{
    this->log = p_log;
    type = core::ClassType::Scene;
    sceneUpdatedTimeStamp = 0;
    lightsEnabled = false;
}

std::shared_ptr<UnifiedId> Scene::getSceneId()
{
    return this->getUniqueId();
}

std::vector<std::shared_ptr<SceneObject>> &Scene::getSceneObject()
{
    std::lock_guard<std::mutex> lock(sceneLock); //Lock is released when leaves scope
    return sceneObjects;
}

std::shared_ptr<Scene> Scene::operator+=(std::shared_ptr<SceneObject> p_sceneObject)
{
    addSceneObject(p_sceneObject);
    return safeDownCast<Scene>();
}

void Scene::addSceneObject(std::shared_ptr<SceneObject> p_sceneObject)
{
    if (p_sceneObject != nullptr)
    {
        std::lock_guard<std::mutex> lock(sceneLock); //Lock is released when leaves scope
        sceneObjects.push_back(p_sceneObject);
        sceneUpdatedTimeStamp++;
    }
}

/// \brief removes the scene object based on scene object id
void Scene::removeSceneObject(std::shared_ptr<SceneObject> p_sceneObject)
{
    if (p_sceneObject != nullptr)
    {
        std::lock_guard<std::mutex> lock(sceneLock); //Lock is released when leaves scope

        for (size_t i = 0; i < sceneObjects.size(); i++)
        {
            if (sceneObjects[i] == p_sceneObject)
            {
                sceneObjects.erase(sceneObjects.begin() + i);
            }
        }

        sceneUpdatedTimeStamp++;
    }
}

/// \brief removes the object from the scene based on its object id
void Scene::removeSceneObject(std::shared_ptr<UnifiedId> p_sceneObjectId)
{
    std::lock_guard<std::mutex> lock(sceneLock); //Lock is released when leaves scope
    short id = p_sceneObjectId->getId();
    if (id >= 0 && id < int(sceneObjects.size()))
    {
        for (size_t i = 0; i < sceneObjects.size(); i++)
        {
            if (sceneObjects[i]->getUniqueId()->getId() == id)
            {
                sceneObjects.erase(sceneObjects.begin() + i);
            }
        }

    }

    sceneUpdatedTimeStamp++;
}

void Scene::copySceneObjects(std::shared_ptr<Scene> p_scene)
{
    p_scene->sceneObjects.clear();

    for (size_t i = 0; i < this->sceneObjects.size(); i++)
    {
        p_scene->sceneObjects.emplace_back(sceneObjects[i]);
    }
}

std::shared_ptr<Scene> Scene::operator=(std::shared_ptr<Scene> p_scene)
{
    copySceneObjects(p_scene);
    return safeDownCast<Scene>();
}

void Scene::initLights()
{
    core::Vec3f casted;

    for (auto light: lights)
    {
        glEnable(light->renderUsage);
        glLightfv(light->renderUsage, GL_AMBIENT, light->lightColorAmbient.toGLColor<GLfloat>());
        glLightfv(light->renderUsage, GL_DIFFUSE, light->lightColorDiffuse.toGLColor<GLfloat>());
        glLightfv(light->renderUsage, GL_SPECULAR, light->lightColorSpecular.toGLColor<GLfloat>());
        glLightf(light->renderUsage, GL_SPOT_EXPONENT, light->spotExp * SMLIGHT_SPOTMAX);
        glLightf(light->renderUsage, GL_SPOT_CUTOFF, light->spotCutOffAngle);
        casted = light->lightPos.getPosition().cast<float>();
        glLightfv(light->renderUsage, GL_POSITION, casted.data());
        casted = light->direction.cast<float>();
        glLightfv(light->renderUsage, GL_SPOT_DIRECTION, casted.data());
    }
}

int Scene::addLight(std::shared_ptr<Light> p_light)
{
    lights.push_back(p_light);
    lights.back()->renderUsage = GL_LIGHT0 + (lights.size() - 1);
    return (lights.size() - 1);
}

void Scene::refreshLights()
{
    core::Vec3f casted;

    for (auto light: lights)
    {
        glEnable(light->renderUsage);
        glLightfv(light->renderUsage, GL_AMBIENT, light->lightColorAmbient.toGLColor<GLfloat>());
        glLightfv(light->renderUsage, GL_DIFFUSE, light->lightColorDiffuse.toGLColor<GLfloat>());
        glLightfv(light->renderUsage, GL_SPECULAR, light->lightColorSpecular.toGLColor<GLfloat>());
        glLightf(light->renderUsage, GL_SPOT_EXPONENT, light->spotExp * SMLIGHT_SPOTMAX);
        glLightf(light->renderUsage, GL_SPOT_CUTOFF, light->spotCutOffAngle);
        casted = light->lightPos.getPosition().cast<float>();
        glLightfv(light->renderUsage, GL_POSITION, casted.data());
        casted = light->direction.cast<float>();
        glLightfv(light->renderUsage, GL_SPOT_DIRECTION, casted.data());
    }
}

void Scene::setLightPos(int p_lightId, LightPos p_pos)
{
    std::shared_ptr<Light> temp = lights.at(p_lightId);
    temp->lightPos = p_pos;
    temp->updateDirection();
}

void Scene::setLightPos(int p_lightId, LightPos p_pos, core::Vec3d p_direction)
{
    std::shared_ptr<Light> temp = lights.at(p_lightId);
    temp->lightPos = p_pos;
    temp->direction = p_direction;
    temp->updateDirection();
}

void Scene::enableLights(bool b)
{
    lightsEnabled = b;
}

void Scene::activateLights()
{
    if (!lightsEnabled)
    {
        return;
    }

    glEnable(GL_LIGHTING);

    for (auto light: lights)
    {
        if (light->isEnabled())
        {
            glEnable(light->renderUsage);
        }
        else
        {
            glDisable(light->renderUsage);
        }
    }
}

void Scene::deactivateLights()
{
    for (auto light: lights)
    {
        glDisable(light->renderUsage);
    }

    glDisable(GL_LIGHTING);
}

void Scene::placeLights()
{
    if (!lightsEnabled)
    {
        return;
    }

    if (!lightsEnabled)
    {
        return;
    }

    for (auto light: lights)
    {
        core::Vec3f casted;
        if (!(light->isEnabled()))
        {
            continue;
        }

        glLightf(light->renderUsage, GL_CONSTANT_ATTENUATION, light->attn_constant);
        glLightf(light->renderUsage, GL_LINEAR_ATTENUATION, light->attn_linear);
        glLightf(light->renderUsage, GL_QUADRATIC_ATTENUATION, light->attn_quadratic);

        casted = light->lightPos.getPosition().cast<float>();
        if (light->lightLocationType == Light::Eye)
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glLightfv(light->renderUsage, GL_POSITION, casted.data());
            glPopMatrix();
        }
        else
        {
            glLightfv(light->renderUsage, GL_POSITION, casted.data());
        }

        casted = light->direction.cast<float>();
        if (light->lightType == Light::Spotlight)
        {
            glLightfv(light->renderUsage, GL_SPOT_DIRECTION, casted.data());
        }
    }
}

void Scene::copySceneToLocal(SceneLocal &p_local)
{
    std::lock_guard<std::mutex> lock(this->sceneLock); //Lock is released when leaves scope

    if(!(this->sceneUpdatedTimeStamp > p_local.sceneUpdatedTimeStamp))
    {
        return;
    }

    p_local.sceneObjects = this->sceneObjects;
    p_local.sceneUpdatedTimeStamp = sceneUpdatedTimeStamp;
}
