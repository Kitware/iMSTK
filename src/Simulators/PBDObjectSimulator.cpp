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

#include "PBDSceneObject.h"
#include "PBDObjectSimulator.h"
#include "Rendering/GLRenderer.h"

void smPBDObjectSimulator::draw()
{
    smObjectSimulator::draw();
    std::shared_ptr<smPBDSurfaceSceneObject> sceneObject;

    for (size_t i = 0; i < objectsSimulated.size(); i++)
    {
        sceneObject = std::static_pointer_cast<smPBDSurfaceSceneObject>(objectsSimulated[i]);
        smGLRenderer::draw(sceneObject->mesh->aabb);
    }
}
smPBDObjectSimulator::smPBDObjectSimulator( std::shared_ptr<smErrorLog> p_errorLog ) : smObjectSimulator( p_errorLog )
{
    this->scheduleGroup.maxTargetFPS = 100;
    this->scheduleGroup.minTargetFPS = 30;
}
void smPBDObjectSimulator::initObject( std::shared_ptr<smPBDSurfaceSceneObject> p_object )
{
    if ( p_object->getFlags().isSimulatorInit )
    {
        return;
    }

    p_object->getLocalVertices().reserve( p_object->mesh->nbrVertices );

    // WARNING: Copying vertices??!!
    p_object->getLocalVertices() = p_object->mesh->vertices;
    p_object->getFlags().isSimulatorInit = true;
}
void smPBDObjectSimulator::initCustom()
{
    core::ClassType type;
    std::shared_ptr<smSceneObject> object;
    std::shared_ptr<smPBDSurfaceSceneObject> pbdSurfaceSceneObject;

    //do nothing for now
    for ( size_t i = 0; i < objectsSimulated.size(); i++ )
    {
        object = objectsSimulated[i];
        type = object->getType();

        switch ( type )
        {
            case core::ClassType::PbdSurfaceSceneObject:
            {
                pbdSurfaceSceneObject = std::static_pointer_cast<smPBDSurfaceSceneObject>(object);
                initObject ( pbdSurfaceSceneObject );
                break;
            }
            default:
                std::cerr << "Unknown class type" << std::endl;

        }
    }
}
void smPBDObjectSimulator::run()
{
    std::shared_ptr<smSceneObject> sceneObj;
    std::shared_ptr<smPBDSurfaceSceneObject> pbdSurfaceSceneObject;
    float dist, lamda;
    smVec3d dirVec, dP;
    int count = 0;
    int a, b;

    beginSim();

    for ( size_t j = 0; j < this->objectsSimulated.size(); j++ )
    {
        sceneObj = this->objectsSimulated[j];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == core::ClassType::PbdSurfaceSceneObject )
        {
            pbdSurfaceSceneObject = std::static_pointer_cast<smPBDSurfaceSceneObject>(sceneObj);
            smStdVector3d &vertices = pbdSurfaceSceneObject->getLocalVertices();

            if ( !pbdSurfaceSceneObject->getFlags().isSimulatorInit )
            {
                initObject( pbdSurfaceSceneObject );
            }

            for ( int i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->exF[i] = smVec3d::Zero();
            }

            for ( int i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->exF[i][1] -= 1.0;
            }


            for ( int i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->V[i] =
                    pbdSurfaceSceneObject->V[i] +
                    ( pbdSurfaceSceneObject->exF[i] -
                      pbdSurfaceSceneObject->V[i] * pbdSurfaceSceneObject->Damp ) *
                    pbdSurfaceSceneObject->dT;

                if ( !pbdSurfaceSceneObject->fixedMass[i] )
                {
                    pbdSurfaceSceneObject->P[i] =
                        vertices[i] + pbdSurfaceSceneObject->V[i] * pbdSurfaceSceneObject->dT;
                }
            }

            count = 0;

            while ( count < 30 )
            {
                for ( int i = 0; i < pbdSurfaceSceneObject->nbrSpr; i++ )
                {
                    a = pbdSurfaceSceneObject->mesh->edges[i].vert[0];
                    b = pbdSurfaceSceneObject->mesh->edges[i].vert[1];
                    dirVec = pbdSurfaceSceneObject->P[a] - pbdSurfaceSceneObject->P[b];
                    dist = dirVec.norm();
                    dirVec = dirVec / dist;
                    lamda = 0.5 * ( dist - pbdSurfaceSceneObject->L0[i] );
                    dP = dirVec * ( lamda * pbdSurfaceSceneObject-> paraK );

                    if ( pbdSurfaceSceneObject->fixedMass[a] && ( !pbdSurfaceSceneObject->fixedMass[b] ) )
                    {
                        pbdSurfaceSceneObject->P[b] = pbdSurfaceSceneObject->P[b] + dP * 2.0;
                    }
                    else if ( ( !pbdSurfaceSceneObject->fixedMass[a] ) && pbdSurfaceSceneObject->fixedMass[b] )
                    {
                        pbdSurfaceSceneObject->P[a] = pbdSurfaceSceneObject-> P[a] - dP * 2.0;
                    }
                    else if ( ( !pbdSurfaceSceneObject->fixedMass[a] ) && ( !pbdSurfaceSceneObject->fixedMass[b] ) )
                    {
                        pbdSurfaceSceneObject->P[a] = pbdSurfaceSceneObject-> P[a] - dP;
                        pbdSurfaceSceneObject->P[b] = pbdSurfaceSceneObject->P[b] + dP;
                    }
                    else
                    {

                    }
                }

                count++;
            }

            for ( int i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->V[i] = ( pbdSurfaceSceneObject->P[i] - vertices[i] ) / pbdSurfaceSceneObject->dT;

                if ( !pbdSurfaceSceneObject->fixedMass[i] )
                {
                    vertices[i] = pbdSurfaceSceneObject->P[i];
                }
            }
        }
    }

    endSim();
}
void smPBDObjectSimulator::syncBuffers()
{
    std::shared_ptr<smSceneObject> sceneObj;
    std::shared_ptr<smPBDSurfaceSceneObject> pbdSurfaceSceneObject;

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == core::ClassType::PbdSurfaceSceneObject )
        {
            // WARNING: Copying array!!??
            pbdSurfaceSceneObject->mesh->vertices = pbdSurfaceSceneObject->getLocalVertices();
            pbdSurfaceSceneObject->mesh->updateTriangleNormals();
            pbdSurfaceSceneObject->mesh->updateVertexNormals();
            pbdSurfaceSceneObject->mesh->updateTriangleAABB();
        }
    }
}
void smPBDObjectSimulator::handleEvent(std::shared_ptr<mstk::Event::smEvent> p_event )
{
    ;

    switch ( p_event->getEventType().eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
        {
            auto keyBoardData =
            std::static_pointer_cast<smKeyboardEventData>(p_event->getEventData());

            if ( keyBoardData->keyBoardKey == smKey::F1 )
            {
                printf( "F1 Keyboard is pressed %c\n", keyBoardData->keyBoardKey );
            }

            break;
        }
        default:
            std::cout << "Unknown event type." << std::endl;
    }
}
