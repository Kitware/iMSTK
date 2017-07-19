/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/
#ifdef iMSTK_USE_ODE

#include "imstkRigidObject.h"
#include "ode/ode.h"

namespace imstk
{

void RigidObject::setup()
{
    // dynamics and collision objects
    dWorldID world;
    dSpaceID space;
    dBodyID body;
    dGeomID geom;
    dMass m;
    dJointGroupID contactgroup;

    dInitODE();
    // create world
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    dWorldSetGravity(world, 0, 0, -0.2);
    dWorldSetCFM(world, 1e-5);
    dCreatePlane(space, 0, 0, 1, 0);
    contactgroup = dJointGroupCreate(0);
    // create object
    body = dBodyCreate(world);
    geom = dCreateSphere(space, 0.5);
    dMassSetSphere(&m, 1, 0.5);
    dBodySetMass(body, &m);
    dGeomSetBody(geom, body);
    // set initial position
    dBodySetPosition(body, 0, 0, 3);
    // run simulation
    char** filename;
    filename = new char*;
    // clean up
    dJointGroupDestroy(contactgroup);
    dSpaceDestroy(space);
    dWorldDestroy(world);
    dCloseODE();
}

} // imstk

#endif
