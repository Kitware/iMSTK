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

#ifndef SMPLANECOLLISIONMODEL_H
#define SMPLANECOLLISIONMODEL_H

#include <memory>

// Eigen include
#include<Eigen/Geometry>

#include "smCore/smGeometry.h"
#include "smCore/smModelRepresentation.h"

class smPlane;

class smPlaneCollisionModel : public smModelRepresentation
{
public:
    using RigidTransform = Eigen::Transform<double, 2, Eigen::Isometry>;

public:
    smPlaneCollisionModel(const smVec3d &p, const smVec3d &n);

    ~smPlaneCollisionModel();

    void draw() override;

    const smVec3d &getNormal() const;

    void setNormal(const smVec3d &normal);

    const smVec3d &getPosition() const;

    const RigidTransform &getTransform() const;

    void setTransform(const RigidTransform &t);

private:
    std::shared_ptr<smPlane> plane;
    RigidTransform transform;
};

#endif // SMPLANECOLLISIONMODEL_H_H
