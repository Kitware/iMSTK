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

#ifndef SMFEMSIMULATOR_H
#define SMFEMSIMULATOR_H
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"

/// \brief Example FEM simulator
class smFemSimulator: public smObjectSimulator, public smEventHandler
{
private:
    smVec3<smFloat> hapticPosition;
    smBool hapticButtonPressed;
    smEventDispatcher *eventDispatcher;

public:
    /// \brief constructor
    smFemSimulator(smErrorLog *p_errorLog): smObjectSimulator(p_errorLog)
    {
        hapticButtonPressed = false;
    }

    /// \brief !!
    void setDispatcher(smEventDispatcher *p_eventDispatcher)
    {
        eventDispatcher = p_eventDispatcher;
    }

protected:
    /// \brief !!
    virtual void beginSim()
    {

    }

    /// \brief !!
    virtual void initCustom()
    {

        smClassType type;
        smSceneObject *object;
        smFemSceneObject *femObject;
        smVec3<smFloat> *newVertices;

        //do nothing for now
        for (smInt i = 0; i < objectsSimulated.size(); i++)
        {
            object = objectsSimulated[i];
            type = object->getType();

            switch (type)
            {
            case SIMMEDTK_SMFEMSCENEOBJECT:
                femObject = (smFemSceneObject*)object;
                object->memBlock->allocate<smVec3<smFloat>>(QString("fem"), femObject->v_mesh->nbrVertices);
                object->memBlock->originaltoLocalBlock(QString("fem"), femObject->v_mesh->vertices, femObject->v_mesh->nbrVertices);
                object->flags.isSimulatorInit = true;
                break;
            }
        }
    }

    /// \brief run the fem simulation in a loop here
    virtual void run()
    {

        smSceneObject *sceneObj;
        smFemSceneObject *femSceneObject;
        smVec3<smFloat> *vertices;
        smVolumeMesh *mesh;
        static smInt pickedIndex = -1;
        static smBool nodePicked = false;
        smFloat minDisp = 99999;
        smEvent *eventForce;
        smHapticInEventData *hapticData;
        static smInt dofNumber = 0;

        beginSim();

        for (smInt i = 0; i < this->objectsSimulated.size(); i++)
        {

            sceneObj = this->objectsSimulated[i];

            //ensure that dummy simulator will work on static scene objects only.
            if (sceneObj->getType() == SIMMEDTK_SMFEMSCENEOBJECT)
            {
                femSceneObject = (smFemSceneObject*)sceneObj;
                mesh = femSceneObject->v_mesh;
                femSceneObject->memBlock->getBlock(QString("fem"), (void**)&vertices);

                if (!hapticButtonPressed)
                {
                    pickedIndex = -1;
                    nodePicked = false;
                }

                if (i == 0 && hapticButtonPressed && nodePicked == false)
                {
                    for (smInt i = 0; i < mesh->nbrVertices; i++)
                    {
                        if ((hapticPosition - mesh->origVerts[i]).module() < minDisp)
                        {
                            pickedIndex = i;
                            nodePicked = true;
                            minDisp = (hapticPosition - mesh->origVerts[i]).module();
                        }
                    }
                }

                if (i == 0 && nodePicked && hapticButtonPressed)
                {
                    dofNumber = femSceneObject->ID(pickedIndex, 0);

                    if (dofNumber != 0)
                    {
                        femSceneObject->pulledNode = pickedIndex;
                        femSceneObject->displacements(dofNumber - 1, 0) = hapticPosition.x - femSceneObject->v_mesh->origVerts[pickedIndex].x;
                        femSceneObject->displacements(dofNumber, 0) = hapticPosition.y - femSceneObject->v_mesh->origVerts[pickedIndex].y;
                        femSceneObject->displacements(dofNumber + 1, 0) = hapticPosition.z - femSceneObject->v_mesh->origVerts[pickedIndex].z;
                    }
                }
                else
                {
                    femSceneObject->pulledNode = 62;
                    smInt dofNumber = femSceneObject->ID(62, 0);
                    femSceneObject->displacements(dofNumber - 1, 0) = 0;
                    femSceneObject->displacements(dofNumber, 0) = 0;
                    femSceneObject->displacements(dofNumber + 1, 0) = 0;
                }

                if (femSceneObject->dynamicFem)
                {
                    for (i = 0; i < 1; i++)
                    {
                        femSceneObject->calculateDisplacements_Dynamic(vertices);
                    }
                }
                else
                {
                    femSceneObject->calculateDisplacements_QStatic(vertices);
                }

                if (i == 0)
                {
                    eventForce = new smEvent();
                    eventForce->eventType = SIMMEDTK_EVENTTYPE_HAPTICIN;
                    hapticData = new smHapticInEventData();
                    hapticData->deviceId = 1;

                    if (nodePicked && dofNumber != 0)
                    {
                        hapticData->force.x = -(hapticPosition.x - femSceneObject->v_mesh->origVerts[pickedIndex].x);
                        hapticData->force.y = -(hapticPosition.y - femSceneObject->v_mesh->origVerts[pickedIndex].y);
                        hapticData->force.z = -(hapticPosition.z - femSceneObject->v_mesh->origVerts[pickedIndex].z);
                        cout << hapticData->force.x << "," << hapticData->force.y << "," << hapticData->force.z << endl;
                    }
                    else
                    {
                        hapticData->force.x = 0;
                        hapticData->force.y = 0;
                        hapticData->force.z = 0;
                    }

                    eventForce->data = hapticData;
                    eventDispatcher->sendEventAndDelete(eventForce);
                }
            }
        }

        endSim();
    }

    /// \brief !!
    void endSim()
    {
        //end the job
    }

    /// \brief synchronize the buffers in the object (do not call by yourself).
    void syncBuffers()
    {

        smSceneObject *sceneObj;
        smFemSceneObject *femObject;
        smVec3<smFloat> *vertices;
        smVolumeMesh *mesh;

        for (smInt i = 0; i < this->objectsSimulated.size(); i++)
        {
            sceneObj = this->objectsSimulated[i];

            //ensure that dummy simulator will work on static scene objects only.
            if (sceneObj->getType() == SIMMEDTK_SMFEMSCENEOBJECT)
            {
                femObject = (smFemSceneObject*)sceneObj;
                mesh = femObject->v_mesh;
                femObject->memBlock->localtoOriginalBlock(QString("fem"), mesh->vertices, mesh->nbrVertices);
                femObject->v_mesh->updateVertexNormals();
            }
        }
    }

    /// \brief !!
    void handleEvent(smEvent *p_event)
    {

        smKeyboardEventData *keyBoardData;
        smHapticOutEventData *hapticEventData;

        switch (p_event->eventType.eventTypeCode)
        {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
            keyBoardData = (smKeyboardEventData*)p_event->data;

            if (keyBoardData->keyBoardKey == Qt::Key_F1)
            {
                printf("F1 Keyboard is pressed %c\n", keyBoardData->keyBoardKey);
            }

            break;

        case SIMMEDTK_EVENTTYPE_HAPTICOUT:
            hapticEventData = (smHapticOutEventData *)p_event->data;

            if (hapticEventData->deviceId == 1)
            {
                hapticPosition.x = hapticEventData->position.x;
                hapticPosition.y = hapticEventData->position.y;
                hapticPosition.z = hapticEventData->position.z;
                hapticButtonPressed = hapticEventData->buttonState[0];
            }

            break;
        }
    }
};

#endif
