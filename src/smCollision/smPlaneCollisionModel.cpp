/*
 * Copyright 2015 Ricardo Ortiz <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "smCollision/smPlaneCollisionModel.h"

smPlaneCollisionModel::smPlaneCollisionModel(const smVec3d& p, const smVec3d& n) : plane(std::make_shared<smPlane>(p, n))
{

}
smPlaneCollisionModel::~smPlaneCollisionModel() {}
void smPlaneCollisionModel::draw()
{
    this->plane->draw();
}
const smVec3d& smPlaneCollisionModel::getNormal() const
{
     //return this->transform.linear() * this->plane->getUnitNormal();
	 return this->plane->getUnitNormal();//temporary fix
}
void smPlaneCollisionModel::setNormal(const smVec3d& normal)
{
    this->plane->setUnitNormal(normal);
}
const smVec3d& smPlaneCollisionModel::getPosition() const
{
    return this->transform * this->plane->getPoint();
}
const smPlaneCollisionModel::RigidTransform& smPlaneCollisionModel::getTransform() const
{
    return this->transform;
}
void smPlaneCollisionModel::setTransform(const smPlaneCollisionModel::RigidTransform& t)
{
    this->transform = t;
}
