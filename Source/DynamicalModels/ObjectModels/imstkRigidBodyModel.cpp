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

#include "imstkRigidBodyModel.h"
#include "imstkOrientedBox.h"
#include "imstkLogger.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
bool
RigidBodyModel::initialize()
{
    m_rigidBodyWorld = RigidBodyWorld::getInstance();

    m_initialState  = std::make_shared<RigidBodyState>();
    m_previousState = std::make_shared<RigidBodyState>();
    m_currentState  = std::make_shared<RigidBodyState>();

    if (m_geometry && isGeometryValid(m_geometry))
    {
        m_initialState->setPosition(m_geometry->getTranslation());
        m_currentState->setPosition(m_geometry->getTranslation());

        const std::string geomType = m_geometry->getTypeName();
        if (geomType == "Sphere")
        {
            createSphere();
        }
        else if (geomType == "OrientedBox")
        {
            createOrientedBox();
        }
        else if (geomType == "Plane")
        {
            createPlane();
        }
        else if (geomType == "SurfaceMesh")
        {
            createMesh();
        }

        /*if (m_pxDynamicActor)
        {
            m_pxDynamicActor->setSolverIterationCounts(100, 100);
        }*/
        return true;
    }
    else
    {
        LOG(WARNING) << "Cannot initialize without valid model geometry";
        return false;
    }
}

void
RigidBodyModel::createSphere()
{
    const auto physxMaterial = m_rigidBodyWorld->m_Physics->createMaterial((PxReal)m_config->m_staticFriction,
        (PxReal)m_config->m_dynamicFriction,
        (PxReal)m_config->m_restitution);

    const auto physics = m_rigidBodyWorld->m_Physics;
    const auto PxScene = m_rigidBodyWorld->m_Scene;

    const auto sphereGeo = std::dynamic_pointer_cast<imstk::Sphere>(m_geometry);

    auto       r     = (PxReal)sphereGeo->getRadius();
    const auto p     = sphereGeo->getPosition() + sphereGeo->getTranslation();
    auto       trans = PxTransform((float)p[0], (float)p[1], (float)p[2]);

    if (m_isStatic)
    {
        m_pxStaticActor = PxCreateStatic(*physics,
                                         trans,
                                         PxSphereGeometry(r),
                                         *physxMaterial);
        if (m_pxStaticActor)
        {
            PxScene->addActor(*m_pxStaticActor);
        }
    }
    else
    {
        PxGeometry s = PxSphereGeometry((PxReal)2. * r);
        m_pxDynamicActor = PxCreateDynamic(*physics,
                                           trans,
                                           s,
                                           *physxMaterial,
                                           1.);
        if (m_pxDynamicActor)
        {
            PxScene->addActor(*m_pxDynamicActor);
        }
    }
}

void
RigidBodyModel::createPlane()
{
    const auto physxMaterial = m_rigidBodyWorld->m_Physics->createMaterial((PxReal)m_config->m_staticFriction,
        (PxReal)m_config->m_dynamicFriction,
        (PxReal)m_config->m_restitution);

    const auto physics = m_rigidBodyWorld->m_Physics;

    const auto planeGeo = std::dynamic_pointer_cast<imstk::Plane>(m_geometry);

    const auto n = planeGeo->getNormal();

    PxTransform trans(PxIdentity);

    if (m_isStatic)
    {
        auto plane = PxPlane(PxVec3(0., 0., 0.), PxVec3((float)n[0], (float)n[1], (float)n[2]));

        m_pxStaticActor = PxCreateStatic(*physics,
                                         PxTransformFromPlaneEquation(plane),
                                         PxPlaneGeometry(),
                                         *physxMaterial);
        if (m_pxStaticActor)
        {
            m_rigidBodyWorld->m_Scene->addActor(*m_pxStaticActor);
        }
    }
    else
    {
        LOG(WARNING) << "Dynamic plane is not supported!";
    }
}

void
RigidBodyModel::createOrientedBox()
{
    const auto physxMaterial = m_rigidBodyWorld->m_Physics->createMaterial((PxReal)m_config->m_staticFriction,
        (PxReal)m_config->m_dynamicFriction,
        (PxReal)m_config->m_restitution);

    const auto physics = m_rigidBodyWorld->m_Physics;
    const auto PxScene = m_rigidBodyWorld->m_Scene;

    const auto obbGeo = std::dynamic_pointer_cast<OrientedBox>(m_geometry);

    Vec3d extents = obbGeo->getExtents();

    const auto  p = obbGeo->getPosition() + obbGeo->getTranslation();
    const Quatd q(obbGeo->getRotation());

    PxTransform trans((float)p[0], (float)p[1], (float)p[2], PxQuat((float)q.x(), (float)q.y(), (float)q.z(), (float)q.w()));

    if (m_isStatic)
    {
        m_pxStaticActor = PxCreateStatic(*physics,
                                         trans,
                                         PxBoxGeometry(
                                             static_cast<PxReal>(extents[0]),
                                             static_cast<PxReal>(extents[1]),
                                             static_cast<PxReal>(extents[2])),
                                         *physxMaterial);
        if (m_pxStaticActor)
        {
            PxScene->addActor(*m_pxStaticActor);
        }
    }
    else
    {
        m_pxDynamicActor = PxCreateDynamic(*physics,
                                           trans,
                                           PxBoxGeometry(
                                               static_cast<PxReal>(extents[0]),
                                               static_cast<PxReal>(extents[1]),
                                               static_cast<PxReal>(extents[2])),
                                           *physxMaterial,
            (PxReal)0.1);

        if (m_pxDynamicActor)
        {
            m_pxDynamicActor->setSleepThreshold(0);
            PxScene->addActor(*m_pxDynamicActor);
        }
    }
}

void
RigidBodyModel::createMesh()
{
    const auto PxPhysics = m_rigidBodyWorld->m_Physics;
    const auto PxScene   = m_rigidBodyWorld->m_Scene;

    const auto meshGeo = std::dynamic_pointer_cast<SurfaceMesh>(m_geometry);

    const auto numVerts     = meshGeo->getNumVertices();
    const auto numTriangles = meshGeo->getNumTriangles();

    const VecDataArray<int, 3>& triVerts = *meshGeo->getTriangleIndices();
    const auto                  vertData = *meshGeo->getVertexPositions();

    PxVec3* vertices = new PxVec3[numVerts];
    PxU32*  indices  = new PxU32[numTriangles * 3];

    for (size_t i = 0; i < numVerts; ++i)
    {
        const Vec3d& v = vertData[i];
        vertices[i] = PxVec3(static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    }

    for (size_t i = 0; i < numTriangles; ++i)
    {
        auto t = triVerts[i];
        indices[3 * i]     = static_cast<PxU32>(t[0]);
        indices[3 * i + 1] = static_cast<PxU32>(t[1]);
        indices[3 * i + 2] = static_cast<PxU32>(t[2]);
    }

    PxTriangleMesh* triMesh = createBV34TriangleMesh(static_cast<PxU32>(numVerts), vertices, static_cast<PxU32>(numTriangles), indices, false, false, false, 4);
    PxTransform     trans(PxIdentity);
    const auto      physxMaterial = PxPhysics->createMaterial((PxReal)m_config->m_staticFriction,
        (PxReal)m_config->m_dynamicFriction,
        (PxReal)m_config->m_restitution);

    // Add mesh actor
    if (m_isStatic)
    {
        m_pxStaticActor = PxPhysics->createRigidStatic(trans);
        PxShape* aConvexShape = PxRigidActorExt::createExclusiveShape(*m_pxStaticActor, PxTriangleMeshGeometry(triMesh), *physxMaterial, PxShapeFlag::eSIMULATION_SHAPE);
        if (!aConvexShape)
        {
            LOG(FATAL) << "Could not create convex shape!";
        }

        PxScene->addActor(*m_pxStaticActor);
    }
    else
    {
        m_pxDynamicActor = PxPhysics->createRigidDynamic(trans);
        if (m_config->m_rigidBodyType == RigidBodyType::Kinematic)
        {
            m_pxDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
        }

        PxShape* aConvexShape = PxRigidActorExt::createExclusiveShape(*m_pxDynamicActor, PxTriangleMeshGeometry(triMesh), *physxMaterial, PxShapeFlag::eSIMULATION_SHAPE);

        if (!aConvexShape)
        {
            LOG(FATAL) << "Could not create convex shape!";
        }

        PxScene->addActor(*m_pxDynamicActor);
    }
}

// \todo: Have to implement update physics geometry
void
RigidBodyModel::updatePhysicsGeometry()
{
    if (!m_isStatic)
    {
        const PxTransform pose = m_pxDynamicActor->getGlobalPose();

        auto Pos_physx = pose.p;
        auto q_physx   = pose.q.getNormalized();
        //auto pos_init  = g->getPosition();

        //g->setRotation(Quatd(q_physx.w, q_physx.x, q_physx.y, q_physx.z));

        /*RigidTransform3d T;
        T.translate(Vec3d(Pos_physx.x, Pos_physx.y, Pos_physx.z) - pos_init);
        T.rotate(Quatd(q_physx.w, q_physx.x, q_physx.y, q_physx.z));
        g->transform(T, Geometry::TransformType::ApplyToData);*/

        m_geometry->setRotation(Quatd(q_physx.w, q_physx.x, q_physx.y, q_physx.z));
        m_geometry->setTranslation(Vec3d(Pos_physx.x, Pos_physx.y, Pos_physx.z));

        this->addForce(m_force, m_forcePos, true);
    }

    m_force = Vec3d(0., 0., 0.);
}

// \todo: Have to implement update physics spring geometry
void
RigidBodyModel::addForce(const Vec3d& force, const Vec3d& pos, bool wakeup)
{
    m_force   += force;
    m_forcePos = pos;
    PxRigidBodyExt::addForceAtLocalPos(*m_pxDynamicActor,
        physx::PxVec3((float)force[0], (float)force[1], (float)force[2]),
        physx::PxVec3((float)pos[0], (float)pos[1], (float)pos[2]),
        PxForceMode::eFORCE, wakeup);
}

void
RigidBodyModel::configure(const std::shared_ptr<RigidBodyConfig> matProperty)
{
    if (matProperty->m_rigidBodyType == RigidBodyType::Static)
    {
        m_isStatic = true;
    }
    else
    {
        m_isStatic = false;
    }
    m_config = matProperty;
}

void
RigidBodyModel::setKinematicTarget(const PxTransform& destination)
{
    if (m_config->m_rigidBodyType == RigidBodyType::Kinematic)
    {
        m_pxDynamicActor->setKinematicTarget(destination);
    }
}

void
RigidBodyModel::setupCommonCookingParams(PxCookingParams& params, bool skipMeshCleanup, bool skipEdgeData)
{
    // we suppress the triangle mesh remap table computation to gain some speed, as we will not need it
// in this snippet
    params.suppressTriangleMeshRemapTable = true;

    // If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. The input mesh must be valid.
    // The following conditions are true for a valid triangle mesh :
    //  1. There are no duplicate vertices(within specified vertexWeldTolerance.See PxCookingParams::meshWeldTolerance)
    //  2. There are no large triangles(within specified PxTolerancesScale.)
    // It is recommended to run a separate validation check in debug/checked builds, see below.

    if (!skipMeshCleanup)
    {
        params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
    }
    else
    {
        params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    }

    // If DISABLE_ACTIVE_EDGES_PREDOCOMPUTE is set, the cooking does not compute the active (convex) edges, and instead
    // marks all edges as active. This makes cooking faster but can slow down contact generation. This flag may change
    // the collision behavior, as all edges of the triangle mesh will now be considered active.
    if (!skipEdgeData)
    {
        params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
    }
    else
    {
        params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
    }
}

PxTriangleMesh*
RigidBodyModel::createBV34TriangleMesh(PxU32 numVertices, const PxVec3* vertices, PxU32 numTriangles, const PxU32* indices,
                                       bool skipMeshCleanup, bool skipEdgeData, bool inserted, const PxU32 numTrisPerLeaf)
{
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count     = numVertices;
    meshDesc.points.data      = vertices;
    meshDesc.points.stride    = sizeof(PxVec3);
    meshDesc.triangles.count  = numTriangles;
    meshDesc.triangles.data   = indices;
    meshDesc.triangles.stride = 3 * sizeof(PxU32);

    PxCookingParams params = m_rigidBodyWorld->m_Cooking->getParams();

    // Create BVH34 mid-phase
    params.midphaseDesc = PxMeshMidPhase::eBVH34;

    // setup common cooking params
    setupCommonCookingParams(params, skipMeshCleanup, skipEdgeData);

    // Cooking mesh with less triangles per leaf produces larger meshes with better runtime performance
    // and worse cooking performance. Cooking time is better when more triangles per leaf are used.
    params.midphaseDesc.mBVH34Desc.numPrimsPerLeaf = numTrisPerLeaf;

    m_rigidBodyWorld->m_Cooking->setParams(params);

#if defined(PX_CHECKED) || defined(PX_DEBUG)
    // If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking.
    // We should check the validity of provided triangles in debug/checked builds though.
    if (skipMeshCleanup)
    {
        PX_ASSERT(gCooking->validateTriangleMesh(meshDesc));
    }
#endif  // DEBUG

    PxU32 meshSize = 0;

    PxTriangleMesh* triMesh = NULL;

    // The cooked mesh may either be saved to a stream for later loading, or inserted directly into PxPhysics.
    if (inserted)
    {
        triMesh = m_rigidBodyWorld->m_Cooking->createTriangleMesh(meshDesc, m_rigidBodyWorld->m_Physics->getPhysicsInsertionCallback());
    }
    else
    {
        PxDefaultMemoryOutputStream outBuffer;
        m_rigidBodyWorld->m_Cooking->cookTriangleMesh(meshDesc, outBuffer);

        PxDefaultMemoryInputData stream(outBuffer.getData(), outBuffer.getSize());
        triMesh = m_rigidBodyWorld->m_Physics->createTriangleMesh(stream);

        meshSize = outBuffer.getSize();
    }

    if (!inserted)
    {
        printf("\t Mesh size: %d \n", meshSize);
    }

    return triMesh;
}
} // imstk