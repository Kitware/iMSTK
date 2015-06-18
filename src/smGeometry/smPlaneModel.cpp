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

#include "smGeometry/smPlaneModel.h"

smPlaneModel::smPlaneModel(const smVec3d& p, const smVec3d& n)
{
    this->plane = std::make_shared<smPlane>(p, n);
    this->transform = RigidTransformType::Identity();
}
smPlaneModel::~smPlaneModel() {}
void smPlaneModel::draw()
{
    this->plane->draw();
}
const smVec3d& smPlaneModel::getNormal() const
{
    return this->transform.linear() * this->plane->getUnitNormal();
}
void smPlaneModel::setNormal(const smVec3d& normal)
{
    this->plane->setUnitNormal(normal);
}
const smVec3d& smPlaneModel::getPosition() const
{
    return this->transform * this->plane->getPoint();
}
const smPlaneModel::RigidTransformType& smPlaneModel::getTransform() const
{
    return this->transform;
}
void smPlaneModel::setTransform(const smPlaneModel::RigidTransformType& t)
{
    this->transform = t;
}

void smPlaneModel::setPlaneModel(const std::shared_ptr<smPlane> &p)
{
    this->plane = p;
}

std::shared_ptr<smPlane> smPlaneModel::getPlaneModel() const
{
    return plane;
}
