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

#include "smSimulators/smToolSimulator.h"

void smToolSimulator::updateTool(smStylusRigidSceneObject *p_tool)
{
    smMatrix44f tempMat, tempMatDevice;
    smMatrix44f mat;
    tree<smMeshContainer*>::pre_order_iterator iter = p_tool->meshes.begin();
    //update the Root node first
    iter.node->data->computeCurrentMatrix();
    tempMat = p_tool->transRot * iter.node->data->tempCurrentMatrix;
    iter.node->data->currentMatrix = tempMat;

    tempMatDevice = p_tool->transRotDevice * iter.node->data->tempCurrentDeviceMatrix;
    iter.node->data->currentDeviceMatrix = tempMatDevice;

    if (p_tool->posTraverseCallbackEnabled && p_tool->updateViewerMatrixEnabled)
    {
        iter.node->data->currentViewerMatrix = tempMat;
        p_tool->posTraverseCallBack(**iter);
    }
    else
    {
        p_tool->posTraverseCallBack(**iter);
    }

    iter.node->data->currentDeviceMatrix = tempMatDevice;
    iter++;

    while (iter != p_tool->meshes.end())
    {
        ///the parent nodes matrix should be accumulated
        iter.node->data->accumulatedMatrix = iter.node->parent->data->currentMatrix;
        iter.node->data->accumulatedDeviceMatrix = iter.node->parent->data->currentDeviceMatrix;
        iter.node->data->computeCurrentMatrix();
        iter.node->data->currentMatrix = iter.node->data->tempCurrentMatrix;
        iter.node->data->currentDeviceMatrix = iter.node->data->tempCurrentDeviceMatrix;

        if (p_tool->posTraverseCallbackEnabled && p_tool->updateViewerMatrixEnabled)
        {
            iter.node->data->currentViewerMatrix = iter.node->data->currentMatrix;
            p_tool->posTraverseCallBack(**iter);
        }
        else
        {
            p_tool->posTraverseCallBack(**iter);
        }

        iter++;
    }

    if (p_tool->posCallBackEnabledForEntireObject)
    {
        p_tool->posTraverseCallBack();
    }
}
smToolSimulator::smToolSimulator( smErrorLog *p_errorLog ) : smObjectSimulator( p_errorLog )
{
}
void smToolSimulator::initCustom()
{
}
void smToolSimulator::run()
{

    smSceneObject *sceneObj;
    smStylusRigidSceneObject *tool;

    while ( true && this->enabled )
    {
        beginSim();

        for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
        {
            sceneObj = this->objectsSimulated[i];

            //ensure that dummy simulator will work on static scene objects only.
            if ( sceneObj->getType() == SIMMEDTK_SMSTYLUSRIGIDSCENEOBJECT )
            {
                tool = static_cast<smStylusRigidSceneObject *>(sceneObj);

                if ( tool->toolEnabled )
                {
                    updateTool( tool );
                }
            }
        }

        endSim();
    }
}
void smToolSimulator::syncBuffers()
{
}
void smToolSimulator::handleEvent( smEvent *p_event )
{
    switch ( p_event->eventType.eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
        {
            smKeyboardEventData *keyBoardData = reinterpret_cast<smKeyboardEventData*>(p_event->data);

            if ( keyBoardData->keyBoardKey == smKey::F1 )
            {
                printf( "F1 Keyboard is pressed %c\n", keyBoardData->keyBoardKey );
            }

            break;
        }
    }
}
