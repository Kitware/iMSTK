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
#include "smSimulators/smFemSimulator.h"
#include "smSimulators/smFemSceneObject.h"
#include "smEvent/smEvent.h"
#include "smEvent/smKeyboardEvent.h"
#include "smEvent/smEventHandler.h"
#include "smEvent/smHapticEvent.h"


smFemSimulator::smFemSimulator( std::shared_ptr<smErrorLog> p_errorLog ) : smObjectSimulator( p_errorLog )
{
    hapticButtonPressed = false;
}
void smFemSimulator::beginSim()
{

}
void smFemSimulator::initCustom()
{
    smClassType type;
    std::shared_ptr<smSceneObject> object;


    //do nothing for now
    for ( size_t i = 0; i < objectsSimulated.size(); i++ )
    {
        object = objectsSimulated[i];
        type = object->getType();

        switch ( type )
        {
            case SIMMEDTK_SMFEMSCENEOBJECT:
            {
                smStdVector3d &vertices = object->getLocalVertices();
                std::shared_ptr<smFemSceneObject> femObject = std::static_pointer_cast<smFemSceneObject>(object);
                vertices.reserve( femObject->v_mesh->nbrVertices );
                // TODO: Copying entire vertex array!!?
                vertices = femObject->v_mesh->vertices;
                object->getFlags().isSimulatorInit = true;
                break;
            }
            default:
                std::cerr << "Unknown class type." << std::endl;
        }
    }
}
void smFemSimulator::run()
{
    static smInt pickedIndex = -1;
    static smBool nodePicked = false;
    smFloat minDisp = 99999;
    static smInt dofNumber = 0;

    beginSim();

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        auto sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if(sceneObj->getType() == SIMMEDTK_SMFEMSCENEOBJECT)
        {
            auto femSceneObject = std::static_pointer_cast<smFemSceneObject>(sceneObj);
            auto mesh = femSceneObject->v_mesh;

            if ( !hapticButtonPressed )
            {
                pickedIndex = -1;
                nodePicked = false;
            }

            if ( i == 0 && hapticButtonPressed && nodePicked == false )
            {
                for ( smInt i = 0; i < mesh->nbrVertices; i++ )
                {
                    if ( ( hapticPosition - mesh->origVerts[i] ).norm() < minDisp )
                    {
                        pickedIndex = i;
                        nodePicked = true;
                        minDisp = ( hapticPosition - mesh->origVerts[i] ).norm();
                    }
                }
            }

            if ( i == 0 && nodePicked && hapticButtonPressed )
            {
                dofNumber = femSceneObject->ID( pickedIndex, 0 );

                if ( dofNumber != 0 )
                {
                    femSceneObject->pulledNode = pickedIndex;
                    femSceneObject->displacements( dofNumber - 1, 0 ) = hapticPosition[0] - femSceneObject->v_mesh->origVerts[pickedIndex][0];
                    femSceneObject->displacements( dofNumber, 0 ) = hapticPosition[1] - femSceneObject->v_mesh->origVerts[pickedIndex][1];
                    femSceneObject->displacements( dofNumber + 1, 0 ) = hapticPosition[2] - femSceneObject->v_mesh->origVerts[pickedIndex][2];
                }
            }
            else
            {
                femSceneObject->pulledNode = 62;
                smInt dofNumber = femSceneObject->ID( 62, 0 );
                femSceneObject->displacements( dofNumber - 1, 0 ) = 0;
                femSceneObject->displacements( dofNumber, 0 ) = 0;
                femSceneObject->displacements( dofNumber + 1, 0 ) = 0;
            }

            if ( femSceneObject->dynamicFem )
            {
                for ( i = 0; i < 1; i++ )
                {
                    femSceneObject->calculateDisplacements_Dynamic(femSceneObject->getLocalVertices());
                }
            }
            else
            {
                femSceneObject->calculateDisplacements_QStatic(femSceneObject->getLocalVertices());
            }

            if ( i == 0 )
            {
                auto hapticEvent = std::make_shared<smtk::Event::smHapticEvent>(1,"Device1");

                if ( nodePicked && dofNumber != 0 )
                {
                    hapticEvent->setForce(femSceneObject->v_mesh->origVerts[pickedIndex]-hapticPosition);
                    std::cout << hapticEvent->getForce() << std::endl;
                }
                else
                {
                    hapticEvent->setForce(smVec3d::Zero());
                }

                eventHandler->triggerEvent(hapticEvent);
            }
        }
    }

    endSim();
}
void smFemSimulator::endSim()
{
    //end the job
}
void smFemSimulator::syncBuffers()
{
    smVolumeMesh *mesh;

    for ( size_t i = 0; i < this->objectsSimulated.size(); i++ )
    {
        std::shared_ptr<smSceneObject> sceneObj = this->objectsSimulated[i];

        //ensure that dummy simulator will work on static scene objects only.
        if ( sceneObj->getType() == SIMMEDTK_SMFEMSCENEOBJECT )
        {
            auto femObject = std::static_pointer_cast<smFemSceneObject>(sceneObj);
            mesh = femObject->v_mesh;
            // WARNING: Copying vertices??
            mesh->vertices = femObject->getLocalVertices();
            femObject->v_mesh->updateVertexNormals();
        }
    }
}
void smFemSimulator::handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event )
{
    if(!this->isListening())
    {
        return;
    }

    auto hapticEvent = std::static_pointer_cast<smtk::Event::smHapticEvent>(p_event);
    if(hapticEvent != nullptr && hapticEvent->getDeviceId() == 1)
    {
        hapticPosition = hapticEvent->getPosition();
        hapticButtonPressed = hapticEvent->getButtonState(0);
        return;
    }

    auto keyboardEvent = std::static_pointer_cast<smtk::Event::smKeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case smtk::Event::smKey::F1:
            {
                std::cout << "F1 Keyboard is pressed " ;//<< keyboardEvent->getKeyPressed() << std::endl;
            }
            default:
                break;
        }
    }

}
