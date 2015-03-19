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

smScene::smScene(smErrorLog *p_log) :
    sceneLocal(SIMMEDTK_MAX_MODULES),
    smCoreClass()
{
    type = SIMMEDTK_SMSCENE;
    this->log = p_log;
    totalObjects = 0;
    referenceCounter = 0;
    test = 0;
    sceneUpdatedTimeStamp = 0;
    lights = new smIndiceArray<smLight*>(SIMMEDTK_VIEWER_MAXLIGHTS);
}

smInt smScene::getSceneId()
{
    return uniqueId.ID;
}

smInt smScene::getTotalObjects()
{
    return totalObjects;
}

std::vector<smSceneObject*> smScene::getSceneObject()
{
    std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
    return sceneObjects;
}

smScene& smScene::operator +=(smSceneObject *p_sceneObject)
{
    addSceneObject(p_sceneObject);
    return *this;
}


void smScene::addSceneObject(smSceneObject *p_sceneObject)
{
    if (p_sceneObject != NULL)
    {
        std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
        sceneObjects.push_back(p_sceneObject);
        totalObjects = sceneObjects.size();
        sceneUpdatedTimeStamp++;
    }
}

/// \brief removes the scene object based on scene object id
void smScene::removeSceneObject(smSceneObject *p_sceneObject)
{
    if (p_sceneObject != NULL)
    {
        std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope

        for (smInt i = 0; i < sceneObjects.size(); i++)
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
void smScene::removeSceneObject(smInt p_sceneObjectId)
{
    std::lock_guard<std::mutex> lock(sceneListLock); //Lock is released when leaves scope
    if (p_sceneObjectId >= 0 && p_sceneObjectId < sceneObjects.size())
    {
        for (smInt i = 0; i < sceneObjects.size(); i++)
        {
            if (sceneObjects[i]->uniqueId == p_sceneObjectId)
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

void smScene::copySceneObjects(smScene*p_scene)
{

    p_scene->sceneObjects.clear();

    for (smInt i = 0; i < this->sceneObjects.size(); i++)
    {
        p_scene->sceneObjects.push_back(sceneObjects[i]);
    }
}

smScene &smScene::operator =(smScene &p_scene)
{

    copySceneObjects(&p_scene);
    return *this;
}


void smScene::initLights()
{
    smIndiceArrayIter<smLight*> iter(lights);
    // Create light components
    for (smInt i = iter.begin(); i < iter.end(); i++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, (smGLFloat)iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, (smGLFloat)iter[i]->spotCutOffAngle);
        glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
    }
}

smInt smScene::addLight(smLight *p_light)
{
    smInt index = lights->add(p_light);
    lights->getByRef(index)->renderUsage = GL_LIGHT0 + index;
    lights->getByRef(index)->activate(true);
    return index;
}

smBool smScene::setLight(smInt p_lightId, smLight *p_light)
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
    smIndiceArrayIter<smLight*> iter(lights);

    for (smInt i = iter.begin(); i < iter.end(); iter++)
    {
        glEnable(iter[i]->renderUsage);
        glLightfv(iter[i]->renderUsage, GL_AMBIENT, iter[i]->lightColorAmbient.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_DIFFUSE, iter[i]->lightColorDiffuse.toGLColor());
        glLightfv(iter[i]->renderUsage, GL_SPECULAR, iter[i]->lightColorSpecular.toGLColor());
        glLightf(iter[i]->renderUsage, GL_SPOT_EXPONENT, iter[i]->spotExp * SMLIGHT_SPOTMAX);
        glLightf(iter[i]->renderUsage, GL_SPOT_CUTOFF, iter[i]->spotCutOffAngle);
        glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
    }
}

smBool smScene::updateLight(smInt p_lightId, smLight *p_light)
{

    p_light->updateDirection();
    return lights->replace(p_lightId, p_light);
}

void smScene::setLightPos(smInt p_lightId, smLightPos p_pos)
{

    smLight *temp;
    temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->updateDirection();
}

void smScene::setLightPos(smInt p_lightId,
                           smLightPos p_pos,
                           smVec3f p_direction)
{

    smLight *temp;
    temp = lights->getByRef(p_lightId);
    temp->lightPos = p_pos;
    temp->direction = p_direction;
    temp->updateDirection();
}


void smScene::enableLights()
{

    static smIndiceArrayIter<smLight*> iter(lights);
    smFloat dir[4];
    static smLightPos defaultPos(0, 0, 0);

    glEnable(GL_LIGHTING);

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

        if (iter[i]->lightLocationType == SIMMEDTK_LIGHTPOS_EYE)
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
            glPopMatrix();
        }
        else
        {
            glLightfv(iter[i]->renderUsage, GL_POSITION, (smGLFloat*)&iter[i]->lightPos);
        }

        if (iter[i]->lightType == SIMMEDTK_LIGHT_SPOTLIGHT)
        {
            glLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, (smGLFloat*)&iter[i]->direction);
        }

        glGetLightfv(iter[i]->renderUsage, GL_SPOT_DIRECTION, dir);
    }
}