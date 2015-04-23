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
    lights = new smIndiceArray<std::shared_ptr<smLight>>(SIMMEDTK_VIEWER_MAXLIGHTS);
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
    smIndiceArrayIter<std::shared_ptr<smLight>> iter(lights);
    // Create light components
    for (smInt i = iter.begin(); i < iter.end(); i++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, iter[i]->spotCutOffAngle);
        casted = iter[i]->lightPos.getPosition().cast<float>();
        glLightfv(iter[i]->renderUsage, GL_POSITION, casted.data());
        casted = iter[i]->direction.cast<float>();
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, casted.data());
    }
}

smInt smScene::addLight(std::shared_ptr<smLight> p_light)
{
    smInt index = lights->add(p_light);
    lights->getByRef(index)->renderUsage = GL_LIGHT0 + index;
    lights->getByRef(index)->activate(true);
    return index;
}

smBool smScene::setLight(smInt p_lightId, std::shared_ptr<smLight> p_light)
{
    smInt index = lights->replace(p_lightId, p_light);

    if (index > 0)
    {
        lights->getByRef(p_lightId)->renderUsage = GL_LIGHT0 + p_lightId;
        return true;
    }
    return false;
}

void smScene::refreshLights()
{
    smIndiceArrayIter<std::shared_ptr<smLight>> iter(lights);

    smVec3f casted;
    for (smInt i = iter.begin(); i < iter.end(); iter++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, iter[i]->spotCutOffAngle);
        casted = iter[i]->lightPos.getPosition().cast<float>();
        glLightfv(iter[i]->renderUsage, GL_POSITION, casted.data());
        casted = iter[i]->direction.cast<float>();
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, casted.data());
    }
}

smBool smScene::updateLight(smInt p_lightId, std::shared_ptr<smLight> p_light)
{
    p_light->updateDirection();
    return lights->replace(p_lightId, p_light);
}

void smScene::setLightPos(smInt p_lightId, smLightPos p_pos)
{
    std::shared_ptr<smLight> temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->updateDirection();
}

void smScene::setLightPos(smInt p_lightId, smLightPos p_pos, smVec3d p_direction)
{
    std::shared_ptr<smLight> temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->direction = p_direction;
    temp->updateDirection();
}

void smScene::enableLights()
{
    static smIndiceArrayIter<std::shared_ptr<smLight>> iter(lights);
    smFloat dir[4];
    static smLightPos defaultPos(0, 0, 0);

    glEnable(GL_LIGHTING);

    smVec3f casted;
    for (smInt i = iter.begin(); i < iter.end(); i++)
    {
        if (iter[i]->isEnabled())
        {
            glEnable(iter[i]->renderUsage);
        }
        else
        {
            glDisable(iter[i]->renderUsage);
        }

        glLightf(iter[i]->renderUsage, GL_CONSTANT_ATTENUATION, iter[i]->attn_constant);
        glLightf(iter[i]->renderUsage, GL_LINEAR_ATTENUATION, iter[i]->attn_linear);
        glLightf(iter[i]->renderUsage, GL_QUADRATIC_ATTENUATION, iter[i]->attn_quadratic);

        casted = iter[i]->lightPos.getPosition().cast<float>();
        if (iter[i]->lightLocationType == SIMMEDTK_LIGHTPOS_EYE)
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glLightfv(iter[i]->renderUsage, GL_POSITION, casted.data());
            glPopMatrix();
        }
        else
        {
            glLightfv(iter[i]->renderUsage, GL_POSITION, casted.data());
        }

        casted = iter[i]->direction.cast<float>();
        if (iter[i]->lightType == SIMMEDTK_LIGHT_SPOTLIGHT)
        {
            glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, casted.data());
        }

        glGetLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, dir);
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

