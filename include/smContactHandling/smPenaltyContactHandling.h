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


#ifndef SMPENALTY_COLLISIONHANDLING_H
#define SMPENALTY_COLLISIONHANDLING_H

// SimMedTK includes
#include "smCollision/smCollisionHandling.h"

class smCollisionPair;

///
/// @brief Penalty based collision handling
///
class smPenaltyCollisionHandling : public smCollisionHandling
{
public:
	smPenaltyCollisionHandling(smContactHandlingType contactHandlingType);
    virtual ~smPenaltyCollisionHandling() {}

private:
    void computeUnilateralContactForces(std::shared_ptr<smCollisionPair> pairs,
                                        std::vector<double>& forceVec);

    void computeBilateralContactForces(std::shared_ptr<smCollisionPair> pairs, 
										std::vector<double>& forceVec_mesh1,
										std::vector<double>& forceVec_mesh2);

};

#endif // SMPENALTY_COLLISIONHANDLING_H
