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
static dWorldID      odeWorld;
static dJointGroupID odeContactgroup;
// dynamics and collision objects
static dSpaceID odeSpace;
static dBodyID  odeBody;
static dGeomID  odeGeom;
static dMass    odeMass;

void
RigidObject::odeNearCallback(void* data, dGeomID o1, dGeomID o2)
{
    dBodyID  b1 = dGeomGetBody(o1);
    dBodyID  b2 = dGeomGetBody(o2);
    dContact contact;
    contact.surface.mode = dContactBounce | dContactSoftCFM;
    // friction parameter
    contact.surface.mu = dInfinity;
    // bounce is the amount of "bouncyness".
    contact.surface.bounce = 0.9;
    // bounce_vel is the minimum incoming velocity to cause a bounce
    contact.surface.bounce_vel = 0.1;
    // constraint force mixing parameter
    contact.surface.soft_cfm = 0.001;
    if (int numc = dCollide(o1, o2, 1, &contact.geom, sizeof(dContact)))
    {
        dJointID c = dJointCreateContact(odeWorld, odeContactgroup, &contact);
        dJointAttach(c, b1, b2);
    }
}

void
RigidObject::simulationStep()
{
    // find collisions and add contact joints
    dSpaceCollide(odeSpace, 0, &odeNearCallback);
    // step the simulation
    dWorldQuickStep(odeWorld, 0.01);
    // remove all contact joints
    dJointGroupEmpty(odeContactgroup);
}

void
RigidObject::getGeometryConfig(imstk::Vec3d& p, imstk::Mat3d& orientation)
{
    const dReal* pos;
    const dReal* R;

    pos = dGeomGetPosition(odeGeom);
    R   = dGeomGetRotation(odeGeom);

    p[0] = (double)pos[0];
    p[1] = (double)pos[2];
    p[2] = (double)pos[1];

    size_t rotPos = 0;
    for (size_t r = 0; r < 3; r++)
    {
        for (size_t c = 0; c < 3; c++, rotPos++)
        {
            orientation(r, c) = (double)R[rotPos];
        }
    }
}

void
RigidObject::initOde()
{
    dInitODE();
}

void
RigidObject::closeOde()
{
    // clean up
    dJointGroupDestroy(odeContactgroup);
    dSpaceDestroy(odeSpace);
    dWorldDestroy(odeWorld);
    dCloseODE();
}

void
RigidObject::setup()
{
    // create world
    odeWorld = dWorldCreate();
    odeSpace = dHashSpaceCreate(0);
    dWorldSetGravity(odeWorld, 0, 0, -0.2);
    dWorldSetCFM(odeWorld, 1e-5);
    dCreatePlane(odeSpace, 0, 0, 1, 0);
    odeContactgroup = dJointGroupCreate(0);
    // create object
    odeBody = dBodyCreate(odeWorld);
    odeGeom = dCreateSphere(odeSpace, 0.5);
    dMassSetSphere(&odeMass, 1, 0.5);
    dBodySetMass(odeBody, &odeMass);
    dGeomSetBody(odeGeom, odeBody);
    // set initial position
    dBodySetPosition(odeBody, 0, 0, 3);
}
} // imstk

#endif
