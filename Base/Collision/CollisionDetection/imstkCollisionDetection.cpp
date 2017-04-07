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

#include "imstkCollisionDetection.h"

#include "imstkUnidirectionalPlaneToSphereCD.h"
#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkSphereToSphereCD.h"
#include "imstkMeshToSphereCD.h"
#include "imstkMeshToPlaneCD.h"
#include "imstkMeshToMeshCD.h"

#include "imstkCollidingObject.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"

#include <g3log/g3log.hpp>

namespace imstk
{

std::shared_ptr<CollisionDetection>
CollisionDetection::make_collision_detection(const Type& type,
                                             std::shared_ptr<CollidingObject> objA,
                                             std::shared_ptr<CollidingObject> objB,
                                             CollisionData &colData)
{
    switch (type)
    {
    case Type::UnidirectionalPlaneToSphere:
    {
        auto plane = std::dynamic_pointer_cast<Plane>(objA->getCollidingGeometry());
        auto sphere = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());

        // Geometries check
        if (plane == nullptr || sphere == nullptr)
        {
            LOG(WARNING) << "CollisionDetection::make_collision_detection error: "
                         << "invalid object geometries for UnidirectionalPlaneToSphere collision detection.";
            return nullptr;
        }
        return std::make_shared<UnidirectionalPlaneToSphereCD>(plane, sphere, colData);
    }
    break;
    case Type::BidirectionalPlaneToSphere:
    {
        auto plane = std::dynamic_pointer_cast<Plane>(objA->getCollidingGeometry());
        auto sphere = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());

        // Geometries check
        if (plane == nullptr || sphere == nullptr)
        {
            LOG(WARNING) << "CollisionDetection::make_collision_detection error: "
                         << "invalid object geometries for BidirectionalPlaneToSphere collision detection.";
            return nullptr;
        }
        return std::make_shared<BidirectionalPlaneToSphere>(plane, sphere, colData);
    }
    break;
    case Type::SphereToSphere:
    {
        auto sphereA = std::dynamic_pointer_cast<Sphere>(objA->getCollidingGeometry());
        auto sphereB = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());

        // Geometries check
        if (sphereA == nullptr || sphereB == nullptr)
        {
            LOG(WARNING) << "CollisionDetection::make_collision_detection error: "
                         << "invalid object geometries for SphereToSphere collision detection.";
            return nullptr;
        }
        return std::make_shared<SphereToSphereCD>(sphereA, sphereB, colData);
    }
    break;
    case Type::MeshToSphere:
    {
        auto mesh = std::dynamic_pointer_cast<Mesh>(objA->getCollidingGeometry());
        auto sphere = std::dynamic_pointer_cast<Sphere>(objB->getCollidingGeometry());

        // Geometries check
        if (mesh == nullptr || sphere == nullptr)
        {
            LOG(WARNING) << "CollisionDetection::make_collision_detection error: "
                << "invalid object geometries for SphereToSphere collision detection.";
            return nullptr;
        }
        return std::make_shared<MeshToSphereCD>(mesh, sphere, colData);
    }
    break;
    case Type::MeshToPlane:
    {
        auto mesh = std::dynamic_pointer_cast<Mesh>(objA->getCollidingGeometry());
        auto plane = std::dynamic_pointer_cast<Plane>(objB->getCollidingGeometry());

        // Geometries check
        if (mesh == nullptr || plane == nullptr)
        {
            LOG(WARNING) << "CollisionDetection::make_collision_detection error: "
                << "invalid object geometries for SphereToSphere collision detection.";
            return nullptr;
        }
        return std::make_shared<MeshToPlaneCD>(mesh, plane, colData);
    }
    break;
    case Type::MeshToMesh:
    {
        auto meshA = std::dynamic_pointer_cast<SurfaceMesh>(objA->getCollidingGeometry());
        auto meshB = std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry());

        // Geometries check
        if (meshA == nullptr || meshB == nullptr)
        {
            LOG(WARNING) << "CollisionDetection::make_collision_detection error: "
                         << "invalid object geometries for MeshToMesh collision detection.";
            return nullptr;
        }
        return std::make_shared<MeshToMeshCD>(meshA, meshB, colData);
    }
    break;
    default:
    {
        LOG(WARNING) << "CollisionDetection::make_collision_detection error: type not implemented.";
        return nullptr;
    }
    }
}

const CollisionDetection::Type&
CollisionDetection::getType() const
{
    return m_type;
}

const CollisionData&
CollisionDetection::getCollisionData() const
{
    return m_colData;
}
}
