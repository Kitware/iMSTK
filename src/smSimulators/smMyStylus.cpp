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

// Eigen includes
#include "Eigen/Geometry"

// SimMedTK includes
#include "smSimulators/smMyStylus.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smSDK.h"
#include "smUtilities/smMatrix.h"

MyStylus::MyStylus(const smString& p_shaft, const smString& p_lower, const smString& p_upper)
{
    angle = 0;
    smMatrix33f rot = Eigen::AngleAxisf(-SM_PI_HALF, smVec3f::UnitX()).matrix();

    smSurfaceMesh *mesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    mesh->loadMesh(p_shaft, SM_FILETYPE_3DS);
    mesh->assignTexture("hookCautery");
    mesh->scale(smVec3f(0.2, 0.2, 0.2));
    mesh->rotate(rot);

    smSurfaceMesh *lowerMesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    lowerMesh->loadMesh(p_lower, SM_FILETYPE_3DS);
    lowerMesh->assignTexture("metal");
    lowerMesh->scale(smVec3f(0.2, 0.2, 0.2));
    lowerMesh->rotate(rot);

    smSurfaceMesh *upperMesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    upperMesh->loadMesh(p_upper, SM_FILETYPE_3DS);
    upperMesh->assignTexture("metal");
    upperMesh->scale(smVec3f(0.2, 0.2, 0.2));
    upperMesh->rotate(rot);

    meshContainer.name = "HookCauteryPivot";
    meshContainer.mesh = mesh;
    meshContainer.posOffsetPos[2] = 2;

    meshContainerLower.name = "HookCauteryLower";
    meshContainerLower.mesh = lowerMesh;
    meshContainerLower.preOffsetPos[2] = 0.0;
    meshContainerLower.posOffsetPos[2] = -3.5;

    meshContainerUpper.name = "HookCauteryUpper";
    meshContainerUpper.mesh = upperMesh;
    meshContainerUpper.preOffsetPos[2] = 0.0;
    meshContainerUpper.posOffsetPos[2] = -3.5;

    addMeshContainer(&meshContainer);
    addMeshContainer(meshContainer.name, &meshContainerLower);
    addMeshContainer(meshContainer.name, &meshContainerUpper);
}

void MyStylus::updateOpenClose()
{
    if ( buttonState[1] )
    {
        if ( angle < 1.0 )
        {
            angle += 0.000004;
        }
        else
        {
            angle = 1.0;
        }
    }

    if ( buttonState[0] )
    {
        if ( angle > 0.00001 )
        {
            angle -= 0.000004;
        }
        else
        {
            angle = 0.0;
        }
    }
    std::cout << angle << "\n";
}

//This function is not fixed for a reason....I'll give you a hint...try to match the brackets
void MyStylus::handleEvent ( smEvent *p_event )
{


    switch ( p_event->eventType.eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        {
            smMeshContainer *containerLower = this->getMeshContainer ( "HookCauteryLower" );
            smMeshContainer *containerUpper = this->getMeshContainer ( "HookCauteryUpper" );
            smHapticOutEventData *hapticEventData =
                reinterpret_cast<smHapticOutEventData *> ( p_event->data );
            if ( hapticEventData->deviceId == this->phantomID )
            {

                transRot = hapticEventData->transform;

                pos = hapticEventData->position;

                vel = hapticEventData->velocity;

                buttonState[0] = hapticEventData->buttonState[0];
                buttonState[1] = hapticEventData->buttonState[1];
                buttonState[2] = hapticEventData->buttonState[2];
                buttonState[3] = hapticEventData->buttonState[3];

                updateOpenClose();

                containerLower->offsetRotX = angle * 25;
                containerUpper->offsetRotX = -angle * 25;
            }
        }
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
        {
            smKeyboardEventData *keyBoardData =
                reinterpret_cast<smKeyboardEventData *> ( p_event->data );
            if ( keyBoardData->keyBoardKey == smKey::Num2 )
            {
                smSDK::getInstance()->getEventDispatcher()
                    ->disableEventHandler ( this, SIMMEDTK_EVENTTYPE_HAPTICOUT );
                this->renderDetail.renderType = this->renderDetail.renderType
                    | SIMMEDTK_RENDER_NONE;
            }

            if ( keyBoardData->keyBoardKey == smKey::Num1 )
            {
                smSDK::getInstance()->getEventDispatcher()
                    ->enableEventHandler ( this, SIMMEDTK_EVENTTYPE_HAPTICOUT );
                this->renderDetail.renderType = this->renderDetail.renderType
                    & ( ~SIMMEDTK_RENDER_NONE );
            }
            break;
        }
        default:
            std::cerr << "Unknoun event type" << std::endl;
            break;
    }
}

HookCautery::HookCautery(const smString& p_pivot)
{
    smMatrix33f rot = Eigen::AngleAxisf(-SM_PI_HALF, smVec3f::UnitX()).matrix();

    smSurfaceMesh *mesh = new smSurfaceMesh(SMMESH_RIGID, NULL);
    mesh->loadMesh(p_pivot, SM_FILETYPE_3DS);
    mesh->assignTexture("metal");
    mesh->scale(smVec3f(0.2, 0.2, 0.2));
    mesh->rotate(rot);

    meshContainer.name = "HookCauteryPivot";
    meshContainer.mesh = mesh;
    meshContainer.posOffsetPos[2] = 2;

    addMeshContainer(&meshContainer);
}

void HookCautery::draw(const smDrawParam &p_param)
{
    smStylusRigidSceneObject::draw(p_param);
}

void HookCautery::handleEvent ( smEvent *p_event )
{
    switch ( p_event->eventType.eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_HAPTICOUT:
        {
            smHapticOutEventData *hapticEventData =
                reinterpret_cast<smHapticOutEventData *> ( p_event->data );

            if ( hapticEventData->deviceId == this->phantomID )
            {
                transRot = hapticEventData->transform;
                pos = hapticEventData->position;
                vel = hapticEventData->velocity;
                buttonState[0] = hapticEventData->buttonState[0];
                buttonState[1] = hapticEventData->buttonState[1];
                buttonState[2] = hapticEventData->buttonState[2];
                buttonState[3] = hapticEventData->buttonState[3];
            }

            break;
        }
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
        {
            smKeyboardEventData *keyBoardData =
                reinterpret_cast<smKeyboardEventData *> ( p_event->data );

            if ( keyBoardData->keyBoardKey == smKey::Num1 )
            {
                smSDK::getInstance()->getEventDispatcher()
                    ->disableEventHandler ( this, SIMMEDTK_EVENTTYPE_HAPTICOUT );
                this->renderDetail.renderType = ( this->renderDetail.renderType
                    | SIMMEDTK_RENDER_NONE );
            }

            if ( keyBoardData->keyBoardKey == smKey::Num2 )
            {
                smSDK::getInstance()->getEventDispatcher()
                    ->enableEventHandler ( this, SIMMEDTK_EVENTTYPE_HAPTICOUT );
                this->renderDetail.renderType = ( this->renderDetail.renderType
                    & ( ~SIMMEDTK_RENDER_NONE ) );
            }

            break;
        }
        default:
        {
            std::cerr << "Unknoun event type" << std::endl;
            break;
        }
    }
}
