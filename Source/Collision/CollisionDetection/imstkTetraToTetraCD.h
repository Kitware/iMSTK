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

#pragma once

// std library
#include <memory>

// imstk
#include "imstkCollisionDetection.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSpatialHashTableSeparateChaining.h"
#include "DeformModel.h"

namespace imstk
{
class CollisionData;

///
/// \class TetraToTetraCD
///
/// \brief Base class for mesh-to-mesh collision detection
///
class TetraToTetraCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    TetraToTetraCD(std::shared_ptr<TetrahedralMesh> meshA,
                   std::shared_ptr<TetrahedralMesh> meshB,
                   std::shared_ptr<CollisionData>   colData);

    ///
    /// \brief Destructor
    ///
    virtual ~TetraToTetraCD() override = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:

    ///
    /// \brief Processes tetrahedrons of either mesh A or B.
    ///
    /// This method goes through tetrahedrons of the supplied mesh (A or B),
    /// and examines vertices currently in the hash table.
    /// It first does rough intersection check using AABB,
    /// and then finer check using barycentric coordinates.
    /// Collisions are added to m_colData
    /// Self collisions and mutual collisions between A and B are supported.
    ///
    /// \param mesh must be either m_meshA or m_meshB
    /// \param idOffset must be 0 for A, and A.getNumVertices() for B
    ///
    void findCollisionsForMeshWithinHashTable(const std::shared_ptr<TetrahedralMesh> mesh, size_t idOffset);

    std::shared_ptr<TetrahedralMesh> m_meshA;     ///> Mesh A
    std::shared_ptr<TetrahedralMesh> m_meshB;     ///> Mesh B
    SpatialHashTableSeparateChaining m_hashTable; ///> Spatial hash table
};
}
