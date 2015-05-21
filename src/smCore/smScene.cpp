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

#include "smCore/smScene.h"
#include "smCore/smSDK.h"


smScene::smScene(std::shared_ptr<smErrorLog> p_log) :
    smCoreClass(),
    sceneLocal(SIMMEDTK_MAX_MODULES)
{
    this->log = p_log;
    type = SIMMEDTK_SMSCENE;
    totalObjects = 0;
    referenceCounter = 0;
    test = 0;
    sceneUpdatedTimeStamp = 0;
}

std::shared_ptr<smUnifiedId> smScene::getSceneId()
{
    return this->getUniqueId();
}

smInt smScene::getTotalObjects()
{
    return totalObjects;
}

std::vector<std::shared_ptr<smSceneObject>> &smScene::getSceneObject()
{
    std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
    return sceneObjects;
}

std::shared_ptr<smScene> smScene::operator+=(std::shared_ptr<smSceneObject> p_sceneObject)
{
    addSceneObject(p_sceneObject);
    return safeDownCast<smScene>();
}

void smScene::addSceneObject(std::shared_ptr<smSceneObject> p_sceneObject)
{
    if (p_sceneObject != nullptr)
    {
        std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
        sceneObjects.push_back(p_sceneObject);
        totalObjects = sceneObjects.size();
        sceneUpdatedTimeStamp++;
    }
}

/// \brief removes the scene object based on scene object id
void smScene::removeSceneObject(std::shared_ptr<smSceneObject> p_sceneObject)
{
    if (p_sceneObject != nullptr)
    {
        std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope

        for (size_t i = 0; i < sceneObjects.size(); i++)
        {
            if (sceneObjects[i] == p_sceneObject)
            {
                sceneObjects.erase(sceneObjects.begin() + i);
                totalObjects = sceneObjects.size();
            }
        }

        sceneUpdatedTimeStamp++;
    }
}

/// \brief removes the object from the scene based on its object id
void smScene::removeSceneObject(std::shared_ptr<smUnifiedId> p_sceneObjectId)
{
    std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
    short id = p_sceneObjectId->getId();
    if (id >= 0 && id < smInt(sceneObjects.size()))
    {
        for (size_t i = 0; i < sceneObjects.size(); i++)
        {
            if (sceneObjects[i]->getUniqueId()->getId() == id)
            {
                sceneObjects.erase(sceneObjects.begin() + i);
                totalObjects = sceneObjects.size();
            }
        }

    }

    sceneUpdatedTimeStamp++;
}

/// \brief
void smScene::addRef()
{
    //Note: might want to replace with with an atomic variable
    std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
    this->referenceCounter++;
}

void smScene::removeRef()
{
    //Note: might want to replace with with an atomic variable
    std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
    this->referenceCounter--;
}

void smScene::copySceneObjects(std::shared_ptr<smScene> p_scene)
{
    p_scene->sceneObjects.clear();

    for (size_t i = 0; i < this->sceneObjects.size(); i++)
    {
        p_scene->sceneObjects.emplace_back(sceneObjects[i]);
    }
}

std::shared_ptr<smScene> smScene::operator=(std::shared_ptr<smScene> p_scene)
{
    copySceneObjects(p_scene);
    return safeDownCast<smScene>();
}

void smScene::initLights()
{
    smVec3f casted;

    for (auto light: lights)
    {
        glEnable(light->renderUsage);
        glLightfv(light->renderUsage, GL_AMBIENT, light->lightColorAmbient.toGLColor());
        glLightfv(light->renderUsage, GL_DIFFUSE, light->lightColorDiffuse.toGLColor());
        glLightfv(light->renderUsage, GL_SPECULAR, light->lightColorSpecular.toGLColor());
        glLightf(light->renderUsage, GL_SPOT_EXPONENT, light->spotExp * SMLIGHT_SPOTMAX);
        glLightf(light->renderUsage, GL_SPOT_CUTOFF, light->spotCutOffAngle);
        casted = light->lightPos.getPosition().cast<float>();
        glLightfv(light->renderUsage, GL_POSITION, casted.data());
        casted = light->direction.cast<float>();
        glLightfv(light->renderUsage, GL_SPOT_DIRECTION, casted.data());
    }
}

smInt smScene::addLight(std::shared_ptr<smLight> p_light)
{
    lights.push_back(p_light);
    lights.back()->renderUsage = GL_LIGHT0 + (lights.size() - 1);
    return (lights.size() - 1);
}

void smScene::refreshLights()
{
    smVec3f casted;

    for (auto light: lights)
    {
        glEnable(light->renderUsage);
        glLightfv(light->renderUsage, GL_AMBIENT, light->lightColorAmbient.toGLColor());
        glLightfv(light->renderUsage, GL_DIFFUSE, light->lightColorDiffuse.toGLColor());
        glLightfv(light->renderUsage, GL_SPECULAR, light->lightColorSpecular.toGLColor());
        glLightf(light->renderUsage, GL_SPOT_EXPONENT, light->spotExp * SMLIGHT_SPOTMAX);
        glLightf(light->renderUsage, GL_SPOT_CUTOFF, light->spotCutOffAngle);
        casted = light->lightPos.getPosition().cast<float>();
        glLightfv(light->renderUsage, GL_POSITION, casted.data());
        casted = light->direction.cast<float>();
        glLightfv(light->renderUsage, GL_SPOT_DIRECTION, casted.data());
    }
}

void smScene::setLightPos(smInt p_lightId, smLightPos p_pos)
{
    std::shared_ptr<smLight> temp = lights.at(p_lightId);
    temp->lightPos = p_pos;
    temp->updateDirection();
}

void smScene::setLightPos(smInt p_lightId, smLightPos p_pos, smVec3d p_direction)
{
    std::shared_ptr<smLight> temp = lights.at(p_lightId);
    temp->lightPos = p_pos;
    temp->direction = p_direction;
    temp->updateDirection();
}

void smScene::enableLights()
{
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

void smScene::disableLights()
{
    for (auto light: lights)
    {
        glDisable(light->renderUsage);
    }

    glDisable(GL_LIGHTING);
}

void smScene::placeLights()
{
    for (auto light: lights)
    {
        smVec3f casted;
        if (!(light->isEnabled()))
        {
            continue;
        }

        glLightf(light->renderUsage, GL_CONSTANT_ATTENUATION, light->attn_constant);
        glLightf(light->renderUsage, GL_LINEAR_ATTENUATION, light->attn_linear);
        glLightf(light->renderUsage, GL_QUADRATIC_ATTENUATION, light->attn_quadratic);

        casted = light->lightPos.getPosition().cast<float>();
        if (light->lightLocationType == SIMMEDTK_LIGHTPOS_EYE)
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
        if (light->lightType == SIMMEDTK_LIGHT_SPOTLIGHT)
        {
            glLightfv(light->renderUsage, GL_SPOT_DIRECTION, casted.data());
        }
    }
}

void smScene::registerForScene(std::shared_ptr<smCoreClass> p_simmedtkObject)
{
    std::shared_ptr<smSceneLocal> local = std::make_shared<smSceneLocal>();
    local->id = p_simmedtkObject->getUniqueId()->getId();
    std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
    copySceneToLocal(local);
    sceneLocalIndex[local->id] = sceneLocal.checkAndAdd(local);
}
void smScene::copySceneToLocal(std::shared_ptr< smSceneLocal > p_local)
{
    p_local->sceneObjects.clear();

    for(size_t i = 0; i < sceneObjects.size(); i++)
    {
        p_local->sceneObjects.push_back(sceneObjects[i]);
    }

    p_local->sceneUpdatedTimeStamp = sceneUpdatedTimeStamp;
}
void smSceneIterator::setScene(std::shared_ptr< smScene > p_scene, std::shared_ptr< smCoreClass > p_core)
{
    std::lock_guard<std::mutex> lock(p_scene->sceneListLock); //Lock is released when leaves scope
    sceneLocal = p_scene->sceneLocal.getByRef(p_scene->sceneLocalIndex[p_core->getUniqueId()->getId()]);

    if(p_scene->sceneUpdatedTimeStamp > sceneLocal->sceneUpdatedTimeStamp)
    {
        p_scene->copySceneToLocal(sceneLocal);
    }

    endIndex = sceneLocal->sceneObjects.size();
    currentIndex = 0;
}
smSceneIterator::smSceneIterator()
{
    currentIndex = endIndex = 0;
    sceneLocal = nullptr;

}
int smSceneIterator::start()
{
    return 0;
}
void smSceneIterator::operator++()
{
    currentIndex++;
}
int smSceneIterator::end()
{
    return endIndex;
}
void smSceneIterator::operator--()
{
    currentIndex--;
}
std::shared_ptr< smSceneObject > smSceneIterator::operator[](int p_index)
{
    return sceneLocal->sceneObjects[p_index];
}
std::shared_ptr< smSceneObject > smSceneIterator::operator*()
{
    return sceneLocal->sceneObjects[currentIndex];
}

