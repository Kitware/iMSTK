/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */

#ifndef SMDUMMYSIMULATOR_H
#define SMDUMMYSIMULATOR_H
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"

/// \brief Example simulator. This dummy simulator works on static scene objects for now.
class smDummySimulator: public smObjectSimulator, public smEventHandler
{

public:
    /// \brief constructor
    smDummySimulator(smErrorLog *p_errorLog): smObjectSimulator(p_errorLog)
    {
    }

protected:
    virtual void beginSim()
    {
        //start the job
    }

    /// \brief !!
    virtual void initCustom()
    {
        smClassType type;
        smSceneObject *object;
        smStaticSceneObject *staticObject;
        smVec3<smFloat> *newVertices;

        //do nothing for now
        for (smInt i = 0; i < objectsSimulated.size(); i++)
        {
            object = objectsSimulated[i];
            type = object->getType();

            switch (type)
            {
            case SIMMEDTK_SMSTATICSCENEOBJECT:
                staticObject = (smStaticSceneObject*)object;
                object->memBlock->allocate<smVec3<smFloat>>(QString("pos"), staticObject->mesh->nbrVertices);
                object->memBlock->originaltoLocalBlock(QString("pos"), staticObject->mesh->vertices, staticObject->mesh->nbrVertices);
                object->flags.isSimulatorInit = true;
                break;
            }
        }
    }

    /// \brief advance the simulator in time in a loop here
    virtual void run()
    {
        smSceneObject *sceneObj;
        smStaticSceneObject *staticSceneObject;
        smVec3<smFloat> *vertices;
        smMesh *mesh;

        beginSim();

        for (smInt i = 0; i < this->objectsSimulated.size(); i++)
        {
            sceneObj = this->objectsSimulated[i];

            //ensure that dummy simulator will work on static scene objects only.
            if (sceneObj->getType() == SIMMEDTK_SMSTATICSCENEOBJECT)
            {
                staticSceneObject = (smStaticSceneObject*)sceneObj;
                mesh = staticSceneObject->mesh;
                staticSceneObject->memBlock->getBlock(QString("pos"), (void**)&vertices);

                for (smInt vertIndex = 0; vertIndex < staticSceneObject->mesh->nbrVertices; vertIndex++)
                {
                    vertices[vertIndex].y = vertices[vertIndex].y + 0.000001;
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
        smStaticSceneObject *staticSceneObject;
        smVec3<smFloat> *vertices;
        smMesh *mesh;

        for (smInt i = 0; i < this->objectsSimulated.size(); i++)
        {
            sceneObj = this->objectsSimulated[i];

            //ensure that dummy simulator will work on static scene objects only.
            if (sceneObj->getType() == SIMMEDTK_SMSTATICSCENEOBJECT)
            {
                staticSceneObject = (smStaticSceneObject*)sceneObj;
                mesh = staticSceneObject->mesh;
                staticSceneObject->memBlock->localtoOriginalBlock(QString("pos"), mesh->vertices, mesh->nbrVertices);
            }
        }
    }

    /// \brief catch events such as key presses and other user inputs
    void handleEvent(smEvent *p_event)
    {
        smKeyboardEventData *keyBoardData;

        switch (p_event->eventType.eventTypeCode)
        {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
            keyBoardData = (smKeyboardEventData*)p_event->data;

            if (keyBoardData->keyBoardKey == Qt::Key_F1)
            {
                printf("F1 Keyboard is pressed %c\n", keyBoardData->keyBoardKey);
            }

            break;
        }
    }
};

#endif
