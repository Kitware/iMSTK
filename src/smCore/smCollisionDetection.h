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

#ifndef SMCOLLSIONDETECTION_H
#define SMCOLLSIONDETECTION_H

// STL includes
#include <memory>

// SimMedTK includes
#include "smCoreClass.h"

class smMesh;
class smCollisionPair;

/// \brief Base class to calculate contact information between two meshes
/// It determines if two meshes are in close proximity and calculates contacts
/// if they are.
class smCollisionDetection: public smCoreClass
{
public:
    smCollisionDetection() {}

    virtual ~smCollisionDetection() {}

    void computeCollision(std::shared_ptr<smCollisionPair> pairs);

private:
    virtual void doComputeCollision(std::shared_ptr<smCollisionPair> pairs) = 0;
};

#endif
