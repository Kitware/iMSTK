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

#include "Simulators/ToolSimulator.h"
#include "Core/Event.h"
#include "Event/KeyboardEvent.h"

void ToolSimulator::updateTool(std::shared_ptr<StylusRigidSceneObject> p_tool)
{
    Matrix44d tempMat, tempMatDevice;
    Matrix44d mat;
    tree<MeshContainer*>::pre_order_iterator iter = p_tool->meshes.begin();
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

ToolSimulator::ToolSimulator( std::shared_ptr<ErrorLog> p_errorLog ) : ObjectSimulator( p_errorLog )
{
}

void ToolSimulator::initCustom()
{
}

void ToolSimulator::run()
{

    while ( true && this->enabled )
    {
        beginSim();

        for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
        {
            auto sceneObj = this->objectsSimulated[i];

            //ensure that dummy simulator will work on static scene objects only.
            if ( sceneObj->getType() == core::ClassType::StylusRigidSceneObject )
            {
                auto tool = std::static_pointer_cast<StylusRigidSceneObject>(sceneObj);

                if ( tool->toolEnabled )
                {
                    updateTool( tool );
                }
            }
        }

        endSim();
    }
}

void ToolSimulator::syncBuffers()
{
}

void ToolSimulator::handleEvent(std::shared_ptr<core::Event> p_event )
{
    if(!this->isListening())
    {
        return;
    }

    auto keyboardEvent = std::static_pointer_cast<event::KeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case event::Key::F1:
            {
                std::cout << "F1 Keyboard is pressed " ;//<< keyboardEvent->getKeyPressed() << std::endl;
            }
            default:
                break;
        }
    }
}