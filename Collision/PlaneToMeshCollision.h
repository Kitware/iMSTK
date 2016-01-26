// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef COLLISION_PLANETOMESHCOLLISION_H
#define COLLISION_PLANETOMESHCOLLISION_H

// iMSTK includes
#include "Core/CollisionDetection.h"

namespace imstk {

class CollisionManager;

///
/// @brief Brute force collision detection between a mesh and a static plane
///
class PlaneToMeshCollision : public CollisionDetection
{
public:
    PlaneToMeshCollision() {}
    virtual ~PlaneToMeshCollision() {}

    PlaneToMeshCollision(const PlaneToMeshCollision &) = delete;

private:
    virtual void doComputeCollision(std::shared_ptr<CollisionManager> pairs) override;

};

}

#endif // COLLISION_PLANETOMESHCOLLISION_H
