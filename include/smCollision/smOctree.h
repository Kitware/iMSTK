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

#ifndef SM_SMOCTREE_H
#define SM_SMOCTREE_H

// SimMedTK includes
#include "smCollision/smSurfaceTreeCell.h"
#include "smCollision/smOctreeCell.h"
#include "smCore/smGeometry.h"
#include "smUtilities/smVec3.h"


/// \brief octree
class smOctree: public smSurfaceTreeCell<smOctree>
{
public:
    /// \brief constructor
    smOctree()
    {
        triagleIndices.clear();
        filled = false;
    }

    smCube cube; ///< cube
    smVec3f originalCubeCenter; ///< original cube center
    smSphere sphere; ///< !!

    std::vector<int> triagleIndices;  ///< triangle indices
    std::set<int> verticesIndices;  ///< vertices indices
    std::vector<float> weights; /// !!

    /// \brief subdivide octree
    inline void subDivide(const int divisionPerAxis, std::vector<smOctreeCell> &p_cells);

    /// \brief !!
    inline bool isCollided(const smVec3f &p_v0, const smVec3f &p_v1, const smVec3f &p_v2);

    /// \brief !!
    inline void expand(const float p_expansion);
};

#endif
