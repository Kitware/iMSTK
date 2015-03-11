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
 
#include "smSimulators/smPBDSceneObject.h"
#include "smSimulators/smPBDObjectSimulator.h"
#include "smRendering/smGLRenderer.h"

void smPBDObjectSimulator::draw(smDrawParam p_params)
{
    smObjectSimulator::draw(p_params);
    smPBDSurfaceSceneObject *sceneObject;

    for (smInt i = 0; i < objectsSimulated.size(); i++)
    {
        sceneObject = (smPBDSurfaceSceneObject*)objectsSimulated[i];
        smGLRenderer::draw(sceneObject->mesh->aabb);
    }
}
smPBDObjectSimulator::smPBDObjectSimulator( smErrorLog *p_errorLog ) : smObjectSimulator( p_errorLog )
{
    this->scheduleGroup.maxTargetFPS = 100;
    this->scheduleGroup.minTargetFPS = 30;
}
void smPBDObjectSimulator::initObject( smPBDSurfaceSceneObject *p_object )
{
    if ( p_object->flags.isSimulatorInit )
    {
        return;
    }

    p_object->localVerts.reserve( p_object->mesh->nbrVertices );
    p_object->localVerts = p_object->mesh->vertices;
    p_object->flags.isSimulatorInit = true;
}
void smPBDObjectSimulator::initCustom()
{
    smClassType type;
    smSceneObject *object;
    smPBDSurfaceSceneObject *pbdSurfaceSceneObject;
    smVec3f *newVertices;

    //do nothing for now
    for ( smInt i = 0; i < objectsSimulated.size(); i++ )
    {
        object = objectsSimulated[i];
        type = object->getType();

        switch ( type )
        {
            case SIMMEDTK_SMPBDSURFACESCENEOBJECT:
                pbdSurfaceSceneObject = ( smPBDSurfaceSceneObject * )object;
                initObject( pbdSurfaceSceneObject );
                break;
        }
    }
}
void smPBDObjectSimulator::run()
{
    smSceneObject *sceneObj;
    smPBDSurfaceSceneObject *pbdSurfaceSceneObject;
    smVec3f *X;
    smMesh *mesh;
    smInt i;
    smFloat dist, lamda;
    smVec3f dirVec, dP;
    smInt count = 0;
    smInt a, b;

    beginSim();

    for ( smInt j = 0; j < this->objectsSimulated.size(); j++ )
    {
        sceneObj = this->objectsSimulated[j];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMPBDSURFACESCENEOBJECT )
        {
            pbdSurfaceSceneObject = ( smPBDSurfaceSceneObject * )sceneObj;

            if ( !pbdSurfaceSceneObject->flags.isSimulatorInit )
            {
                initObject( pbdSurfaceSceneObject );
            }

            mesh = pbdSurfaceSceneObject->mesh;

            for ( i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->exF[i] = smVec3f::Zero();
            }

            for ( i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->exF[i][1] -= 1.0;
            }

            for ( i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->V[i] =
                    pbdSurfaceSceneObject->V[i] +
                    ( pbdSurfaceSceneObject->exF[i] -
                      pbdSurfaceSceneObject->V[i] * pbdSurfaceSceneObject->Damp ) *
                    pbdSurfaceSceneObject->dT;

                if ( !pbdSurfaceSceneObject->fixedMass[i] )
                {
                    pbdSurfaceSceneObject->P[i] =
                        pbdSurfaceSceneObject->localVerts[i] +
                        pbdSurfaceSceneObject->V[i] * pbdSurfaceSceneObject->dT;
                }
            }

            count = 0;

            while ( count < 30 )
            {
                for ( i = 0; i < pbdSurfaceSceneObject->nbrSpr; i++ )
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

            for ( i = 0; i < pbdSurfaceSceneObject->nbrMass; i++ )
            {
                pbdSurfaceSceneObject->V[i] = ( pbdSurfaceSceneObject->P[i] - pbdSurfaceSceneObject->localVerts[i] ) / pbdSurfaceSceneObject->dT;

                if ( !pbdSurfaceSceneObject->fixedMass[i] )
                {
                    pbdSurfaceSceneObject->localVerts[i] = pbdSurfaceSceneObject->P[i];
                }
            }
        }
    }

    endSim();
}
void smPBDObjectSimulator::syncBuffers()
{
    smSceneObject *sceneObj;
    smPBDSurfaceSceneObject *pbdSurfaceSceneObject;
    smVec3f *vertices;

    for ( smInt i = 0; i < this->objectsSimulated.size(); i++ )
    {
        sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMPBDSURFACESCENEOBJECT )
        {
            pbdSurfaceSceneObject = ( smPBDSurfaceSceneObject * )sceneObj;
            pbdSurfaceSceneObject->mesh->vertices = pbdSurfaceSceneObject->localVerts;
            pbdSurfaceSceneObject->mesh->updateTriangleNormals();
            pbdSurfaceSceneObject->mesh->updateVertexNormals();
            pbdSurfaceSceneObject->mesh->updateTriangleAABB();
        }
    }
}
void smPBDObjectSimulator::handleEvent( smEvent *p_event )
{
    smKeyboardEventData *keyBoardData;

    switch ( p_event->eventType.eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
            keyBoardData = ( smKeyboardEventData * )p_event->data;

            if ( keyBoardData->keyBoardKey == smKey::F1 )
            {
                printf( "F1 Keyboard is pressed %c\n", keyBoardData->keyBoardKey );
            }

            break;
    }
}
