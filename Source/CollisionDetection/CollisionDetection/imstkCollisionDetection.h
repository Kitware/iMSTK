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

#include <memory>

namespace imstk
{
struct CollisionData;

class CollidingObject;
class ComputeNode;
class OctreeBasedCD;
class Geometry;

///
/// \class CollisionDetection
///
/// \brief Base class for all collision detection classes
///
class CollisionDetection
{
public:

    ///
    /// \brief Type of the collision detection
    ///
    enum class Type
    {
        // Points to objects
        PointSetToSphere,
        PointSetToPlane,
        PointSetToCapsule,
        PointSetToSpherePicking,
        PointSetToSurfaceMesh,

        // Mesh to mesh (mesh to analytical object = mesh vertices to analytical object)
        SurfaceMeshToSurfaceMesh,
        SurfaceMeshToSurfaceMeshCCD,
        VolumeMeshToVolumeMesh,
        MeshToMeshBruteForce,

        // Analytical object to analytical object
        UnidirectionalPlaneToSphere,
        BidirectionalPlaneToSphere,
        SphereToCylinder,
        SphereToSphere,

        // Image based CD
        SignedDistanceField,

        Custom
    };

    ///
    /// \brief Constructor
    ///
    CollisionDetection(const Type& type, std::shared_ptr<CollisionData> colData);
    CollisionDetection() = delete;

    ///
    /// \brief Destructor for base class
    ///
    virtual ~CollisionDetection() = default;

    ///
    /// \brief Detect collision and compute collision data (pure virtual)
    ///
    virtual void computeCollisionData() = 0;

    ///
    /// \brief Returns collision detection type
    ///
    const Type& getType() const { return m_type; }

    ///
    /// \brief Returns collision data
    ///
    const std::shared_ptr<CollisionData> getCollisionData() const { return m_colData; }

    ///
    /// \brief Returns computational node
    ///
    std::shared_ptr<ComputeNode> getComputeNode() const { return m_computeNode; }

    ///
    /// \brief Update the intrernal octree, preparing for collision detection
    ///
    static void updateInternalOctreeAndDetectCollision();

    ///
    /// \brief Reset the internal octree, clearing all geometry data and collision pairs from it
    ///
    static void clearInternalOctree();

    ///
    /// \brief Add the geometry into the background octree for collision detection
    /// \todo Add line primitive geometry
    ///
    static void addCollisionPairToOctree(const std::shared_ptr<Geometry>&      geomA,
                                         const std::shared_ptr<Geometry>&      geomB,
                                         const CollisionDetection::Type        collisionType,
                                         const std::shared_ptr<CollisionData>& collisionData);

protected:
    Type m_type = Type::Custom;                             ///> Collision detection algorithm type
    std::shared_ptr<CollisionData> m_colData     = nullptr; ///> Collision data
    std::shared_ptr<ComputeNode>   m_computeNode = nullptr; ///> Computational node to execute the detection

    /// Static octree for collision detection
    /// This octree is valid throughout the lifetime of the program
    /// and will serve as a background mean to detect collision between geometries
    static std::shared_ptr<OctreeBasedCD> s_OctreeCD;
};
}
