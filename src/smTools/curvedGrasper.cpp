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

#include "smTools/curvedGrasper.h"
#include "smCore/smSDK.h"

void curvedGrasper::draw(smDrawParam p_params)
{

    float matrix[16];
    smStylusRigidSceneObject::draw(p_params);
    smMeshContainer *containerLower = this->getMeshContainer("curvedGrasperLower");
    glPushMatrix();
    containerLower->currentMatrix.getMatrixForOpenGL(matrix);
    glMultMatrixf(matrix);
    glPopMatrix();
}

curvedGrasper::curvedGrasper(smInt p_PhantomID,
                             const smString& p_pivotModelFileName,
                             const smString& p_lowerModelFileName,
                             const smString& p_upperModelFileName)
{

    angle = 0;
    maxangle = 10 * 3.14 / 360;
    this->phantomID = p_PhantomID;
    smMatrix33<smFloat> rot;

    mesh_pivot = new smSurfaceMesh(SMMESH_RIGID, NULL);
    mesh_pivot->loadMesh(p_pivotModelFileName, SM_FILETYPE_3DS);
    mesh_pivot->scale(smVec3<smFloat>(0.5, 0.5, 0.5));
    rot.rotAroundY(-SM_PI_HALF);
    mesh_pivot->rotate(rot);
    rot.rotAroundZ(-SM_PI_HALF);
    mesh_pivot->rotate(rot);

    mesh_upperJaw = new smSurfaceMesh(SMMESH_RIGID, NULL);
    mesh_upperJaw->loadMesh(p_upperModelFileName, SM_FILETYPE_3DS);
    mesh_upperJaw->scale(smVec3<smFloat>(0.5, 0.5, 0.5));
    rot.rotAroundY(-SM_PI_HALF);
    mesh_upperJaw->rotate(rot);
    rot.rotAroundZ(-SM_PI_HALF);
    mesh_upperJaw->rotate(rot);

    mesh_lowerJaw = new smSurfaceMesh(SMMESH_RIGID, NULL);
    mesh_lowerJaw->loadMesh(p_lowerModelFileName, SM_FILETYPE_3DS);
    mesh_lowerJaw->scale(smVec3<smFloat>(0.5, 0.5, 0.5));
    rot.rotAroundY(-SM_PI_HALF);
    mesh_lowerJaw->rotate(rot);
    rot.rotAroundZ(-SM_PI_HALF);
    mesh_lowerJaw->rotate(rot);

    meshContainer_pivot.name = "curvedGrasperPivot";
    meshContainer_pivot.mesh = mesh_pivot;

    meshContainer_upperJaw.name = "curvedGrasperUpper";
    meshContainer_upperJaw.mesh = mesh_upperJaw;

    meshContainer_lowerJaw.name = "curvedGrasperLower";
    meshContainer_lowerJaw.mesh = mesh_lowerJaw;

    //for interface
#ifdef smNIUSB6008DAQ
    NIUSB6008pipeReg.regType = SIMMEDTK_PIPE_BYREF;
    NIUSB6008pipeReg.listenerObject = this->mesh_lowerJaw;
#endif

    DAQdataID = 0;
}

void curvedGrasper::handleEvent(smEvent *p_event)
{

    smHapticOutEventData *hapticEventData;
    smKeyboardEventData *keyBoardData;
    smMatrix44d godPosMat;
    smMeshContainer *containerLower = this->getMeshContainer("curvedGrasperLower");
    smMeshContainer *containerUpper = this->getMeshContainer("curvedGrasperUpper");

    switch (p_event->eventType.eventTypeCode)
    {
    case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        hapticEventData = (smHapticOutEventData *)p_event->data;

        if (hapticEventData->deviceId == this->phantomID)
        {
            smVec3d pos1 = hapticEventData->transform.getColumn(3);
            godPosMat = hapticEventData->transform;
            transRot = godPosMat;
            pos = hapticEventData->position;
            vel = hapticEventData->velocity;
            buttonState[0] = hapticEventData->buttonState[0];
            buttonState[1] = hapticEventData->buttonState[1];
            updateOpenClose();
            //buttons
            containerLower->offsetRotY =  angle / 360.0; //angle*maxangle;
            containerUpper->offsetRotY =  -angle / 360.0; //-angle*maxangle;
        }

        break;

    case SIMMEDTK_EVENTTYPE_KEYBOARD:
        keyBoardData = (smKeyboardEventData*)p_event->data;

        if (keyBoardData->keyBoardKey == smKey::Num1)
        {
            smSDK::getInstance()->getEventDispatcher()->disableEventHandler(this, SIMMEDTK_EVENTTYPE_HAPTICOUT);
            this->renderDetail.renderType = this->renderDetail.renderType | SIMMEDTK_RENDER_NONE;
        }

        if (keyBoardData->keyBoardKey == smKey::Num2)
        {
            smSDK::getInstance()->getEventDispatcher()->enableEventHandler(this, SIMMEDTK_EVENTTYPE_HAPTICOUT);
            this->renderDetail.renderType = this->renderDetail.renderType & (~SIMMEDTK_RENDER_NONE);
        }

        break;
    }
}

void curvedGrasper::updateOpenClose()
{

    if (buttonState[0])
    {
        angle -= 0.05;

        if (angle < 0.0)
        {
            angle = 0.0;
        }
    }

    if (buttonState[1])
    {
        angle += 0.05;

        if (angle > 30.0)
        {
            angle = 30.0;
        }
    }

    smFloat tF;
#ifdef smNIUSB6008DAQ
    NIUSB6008Data *NI_Data;

    if (NIUSB6008pipeReg.data.nbrElements > 0)
    {
        NI_Data = (NIUSB6008Data  *)NIUSB6008pipeReg.data.dataLocation;

        if (NI_Data->on)
        {
            angle = NI_Data->value[DAQdataID] * 30.0;
        }
    }

#endif
}
