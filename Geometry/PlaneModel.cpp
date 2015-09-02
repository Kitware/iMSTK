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

#include "Geometry/PlaneModel.h"

PlaneModel::PlaneModel(const core::Vec3d& p, const core::Vec3d& n)
{
    this->plane = std::make_shared<Plane>(p, n);
    this->transform = RigidTransformType::Identity();

    this->setRenderDelegate(
        Factory<RenderDelegate>::createConcreteClassForGroup(
            "StaticSceneObjectRenderDelegate",RenderDelegate::RendererType::VTK));

}
PlaneModel::~PlaneModel() {}
void PlaneModel::draw()
{
    this->plane->draw();
}
const core::Vec3d& PlaneModel::getSurfaceNormal() const
{
    return this->transform.linear() * this->plane->getUnitNormal();
}
void PlaneModel::setNormal(const core::Vec3d& normal)
{
    this->plane->setUnitNormal(normal);
}
const core::Vec3d& PlaneModel::getPosition() const
{
  // NB: This static variable makes the function thread-unsafe.
  static core::Vec3d result;
  result = this->transform * this->plane->getPoint();
  return result;
}
const PlaneModel::RigidTransformType& PlaneModel::getTransform() const
{
    return this->transform;
}
void PlaneModel::setTransform(const PlaneModel::RigidTransformType& t)
{
    this->transform = t;
}

void PlaneModel::setPlaneModel(const std::shared_ptr<Plane> &p)
{
    this->plane = p;
}

std::shared_ptr<Plane> PlaneModel::getPlaneModel() const
{
    return plane;
}
