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

// SimMedTK includes
#include "smSimulators/smDummySimulator.h"

smDummySimulator::smDummySimulator( smErrorLog *p_errorLog ) : smObjectSimulator( p_errorLog )
{
}
void smDummySimulator::beginSim()
{
    //start the job
}
void smDummySimulator::initCustom()
{
    smClassType type;
    smSceneObject *object;
    smStaticSceneObject *staticObject;
    smVec3f *newVertices;

    //do nothing for now
    for ( smInt i = 0; i < objectsSimulated.size(); i++ )
    {
        object = objectsSimulated[i];
        type = object->getType();

        switch ( type )
        {
            case SIMMEDTK_SMSTATICSCENEOBJECT:
                staticObject = ( smStaticSceneObject * )object;
                object->localVerts.reserve( staticObject->mesh->nbrVertices );
                object->localVerts = staticObject->mesh->vertices;
                object->flags.isSimulatorInit = true;
                break;
        }
    }
}
void smDummySimulator::run()
{
    smSceneObject *sceneObj;
    smStaticSceneObject *staticSceneObject;
    smVec3f *vertices;
    smMesh *mesh;

    beginSim();

    for ( smInt i = 0; i < this->objectsSimulated.size(); i++ )
    {
        sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMSTATICSCENEOBJECT )
        {
            staticSceneObject = ( smStaticSceneObject * )sceneObj;
            mesh = staticSceneObject->mesh;

            for ( smInt vertIndex = 0; vertIndex < staticSceneObject->mesh->nbrVertices; vertIndex++ )
            {
                staticSceneObject->localVerts[vertIndex].y = staticSceneObject->localVerts[vertIndex].y + 0.000001;
            }
        }
    }

    endSim();
}
void smDummySimulator::endSim()
{
    //end the job
}
void smDummySimulator::syncBuffers()
{
    smSceneObject *sceneObj;
    smStaticSceneObject *staticSceneObject;
    smMesh *mesh;

    for ( smInt i = 0; i < this->objectsSimulated.size(); i++ )
    {
        sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMSTATICSCENEOBJECT )
        {
            staticSceneObject = ( smStaticSceneObject * )sceneObj;
            mesh = staticSceneObject->mesh;
            mesh->vertices = staticSceneObject->localVerts;
        }
    }
}
void smDummySimulator::handleEvent( smEvent *p_event )
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
