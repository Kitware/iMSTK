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
#include "Simulators/SceneObjectDeformable.h"
#include "Core/Factory.h"

SceneObjectDeformable::SceneObjectDeformable() :
    renderSecondaryMesh(false),
    topologyAltered(false),
    pulledVertex(-1),
    timestepCounter(0),
    subTimestepCounter(0)
{
  this->setRenderDelegate(
    Factory<RenderDelegate>::createSubclass(
      "RenderDelegate", "SceneObjectDeformableRenderDelegate"));
}

SceneObjectDeformable::~SceneObjectDeformable()
{
}

void SceneObjectDeformable::applyContactForces()
{
    if (f_contact.size() != 0)
    {
        for (size_t i = 0; i < f_contact.size(); i++)
        {
            f_ext[i] += f_contact[i];
        }
    }
}

void SceneObjectDeformable::setContactForcesToZero()
{
    f_contact.assign(f_contact.size(), 0.0);
}

void SceneObjectDeformable::setContactForceOfNodeWithDofID(const int dofID,
                                                             const core::Vec3d force)
{
    f_contact[dofID] = force(0);
    f_contact[dofID + 1] = force(1);
    f_contact[dofID + 2] = force(2);
}

core::Vec3d SceneObjectDeformable::getVelocityOfNodeWithDofID(const int dofID) const
{
    core::Vec3d vel(uvel[dofID], uvel[dofID + 1], uvel[dofID + 2]);

    return vel;
}

core::Vec3d SceneObjectDeformable::getDisplacementOfNodeWithDofID(const int dofID) const
{
    core::Vec3d disp(u[dofID], u[dofID + 1], u[dofID + 2]);

    return disp;
}

core::Vec3d SceneObjectDeformable::getAccelerationOfNodeWithDofID(const int dofID) const
{
    core::Vec3d accn(uaccel[dofID], uaccel[dofID + 1], uaccel[dofID + 2]);

    return accn;
}

int SceneObjectDeformable::getNumNodes() const
{
    return numNodes;
}

int SceneObjectDeformable::getNumTotalDof() const
{
    return numTotalDOF;
}

int SceneObjectDeformable::getNumDof() const
{
    return numDOF;
}

int SceneObjectDeformable::getNumFixedNodes() const
{
    return numFixedNodes;
}

int SceneObjectDeformable::getNumFixedDof() const
{
    return numFixedDof;
}

void SceneObjectDeformable::setRenderDetail(const std::shared_ptr<RenderDetail> &r)
{
    primarySurfaceMesh->setRenderDetail(r);

    if (secondarySurfaceMesh != nullptr)
    {
        secondarySurfaceMesh->setRenderDetail(r);
    }
}

void SceneObjectDeformable::setRenderSecondaryMesh()
{
    if (secondarySurfaceMesh != nullptr)
    {
        renderSecondaryMesh = true;
    }
    else
    {
        std::cout << "Secondary rendering mesh is not initilized! Cannot render secondary mesh\n";
    }
}

void SceneObjectDeformable::setRenderPrimaryMesh()
{
    renderSecondaryMesh = false;
}

std::shared_ptr<SurfaceMesh> SceneObjectDeformable::getPrimarySurfaceMesh() const
{
    return primarySurfaceMesh;
}

std::shared_ptr<SurfaceMesh> SceneObjectDeformable::getSecondarySurfaceMesh() const
{
    return secondarySurfaceMesh;
}
