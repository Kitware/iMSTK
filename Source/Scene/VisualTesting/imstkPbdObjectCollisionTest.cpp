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

#include "imstkCamera.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionDataDebugObject.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkDirectionalLight.h"
#include "imstkImplicitGeometry.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPlane.h"
#include "imstkPointwiseMap.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTestingUtils.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"
#include "imstkVisualTestingUtils.h"

using namespace imstk;

///
/// \brief Creates triangle grid geometry
/// \param cloth width (x), height (z)
/// \param cloth dimensions/divisions
/// \param center of tissue/translation control
/// \param tex coord scale
///
static std::shared_ptr<SurfaceMesh>
makeTriangleGrid(const Vec2d& size,
                 const Vec2i& dim,
                 const Vec3d& center,
                 const Quatd& orientation)
{
    auto                     verticesPtr = std::make_shared<VecDataArray<double, 3>>(dim[0] * dim[1]);
    VecDataArray<double, 3>& vertices    = *verticesPtr;
    const Vec3d              size3       = Vec3d(size[0], 0.0, size[1]);
    const Vec3i              dim3 = Vec3i(dim[0], 0, dim[1]);
    Vec3d                    dx   = size3.cwiseQuotient((dim3 - Vec3i(1, 0, 1)).cast<double>());
    dx[1] = 0.0;
    int iter = 0;
    for (int y = 0; y < dim[1]; y++)
    {
        for (int x = 0; x < dim[0]; x++, iter++)
        {
            vertices[iter] = Vec3i(x, 0, y).cast<double>().cwiseProduct(dx) + center - size3 * 0.5;
        }
    }

    // Add connectivity data
    auto                  indicesPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>& indices    = *indicesPtr;
    for (int y = 0; y < dim[1] - 1; y++)
    {
        for (int x = 0; x < dim[0] - 1; x++)
        {
            const int index1 = y * dim[0] + x;
            const int index2 = index1 + dim[0];
            const int index3 = index1 + 1;
            const int index4 = index2 + 1;

            // Interleave [/][\]
            if (x % 2 ^ y % 2)
            {
                indices.push_back(Vec3i(index1, index2, index3));
                indices.push_back(Vec3i(index4, index3, index2));
            }
            else
            {
                indices.push_back(Vec3i(index2, index4, index1));
                indices.push_back(Vec3i(index4, index3, index1));
            }
        }
    }

    auto triMesh = std::make_shared<SurfaceMesh>();
    triMesh->initialize(verticesPtr, indicesPtr);
    triMesh->rotate(orientation, Geometry::TransformType::ApplyToData);
    return triMesh;
}

///
/// \brief Creates a tetraheral grid
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of grid
///
static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center,
            const Quatd& orientation)
{
    auto                     verticesPtr = std::make_shared<VecDataArray<double, 3>>(dim[0] * dim[1] * dim[2]);
    VecDataArray<double, 3>& vertices    = *verticesPtr;
    const Vec3d              dx   = size.cwiseQuotient((dim - Vec3i(1, 1, 1)).cast<double>());
    int                      iter = 0;
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++, iter++)
            {
                vertices[iter] = Vec3i(x, y, z).cast<double>().cwiseProduct(dx) - size * 0.5 + center;
            }
        }
    }

    // Add connectivity data
    auto                  indicesPtr = std::make_shared<VecDataArray<int, 4>>();
    VecDataArray<int, 4>& indices    = *indicesPtr;
    for (int z = 0; z < dim[2] - 1; z++)
    {
        for (int y = 0; y < dim[1] - 1; y++)
        {
            for (int x = 0; x < dim[0] - 1; x++)
            {
                int cubeIndices[8] =
                {
                    x + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * (z + 1))
                };

                // Alternate the pattern so the edges line up on the sides of each voxel
                if ((z % 2 ^ x % 2) ^ y % 2)
                {
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[7], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[2], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[7], cubeIndices[5], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[2], cubeIndices[0], cubeIndices[5]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[6], cubeIndices[7], cubeIndices[5]));
                }
                else
                {
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[3], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[6], cubeIndices[2], cubeIndices[1]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[6], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[3], cubeIndices[1], cubeIndices[4]));
                }
            }
        }
    }

    // Ensure correct windings
    for (int i = 0; i < indices.size(); i++)
    {
        if (tetVolume(vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]]) < 0.0)
        {
            std::swap(indices[i][0], indices[i][2]);
        }
    }

    auto tetMesh = std::make_shared<TetrahedralMesh>();
    tetMesh->initialize(verticesPtr, indicesPtr);
    tetMesh->rotate(orientation, Geometry::TransformType::ApplyToData);
    return tetMesh;
}

///
/// \brief Creates tetrahedral tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTetTissueObj(const std::string& name,
                 const Vec3d& size, const Vec3i& dim, const Vec3d& center,
                 const Quatd& orientation,
                 bool useTetCollisionGeometry)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh = makeTetGrid(size, dim, center, orientation);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    // Use FEMTet constraints (42k - 85k for tissue, but we want
    // something much more stretchy to wrap)
    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.45; // 0.48 for tissue
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
    /* pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 0.01);
     pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.4);*/
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.01;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_viscousDampingCoeff = 0.025;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(tetMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(0.5);

    // Setup the Object
    tissueObj->setPhysicsGeometry(tetMesh);
    if (useTetCollisionGeometry)
    {
        tissueObj->setVisualGeometry(tetMesh);
        tissueObj->setCollidingGeometry(tetMesh);
    }
    else
    {
        std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();
        tissueObj->setVisualGeometry(surfMesh);
        tissueObj->setCollidingGeometry(surfMesh);
        tissueObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(tetMesh, surfMesh));
    }
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

///
/// \brief Creates thin tissue object
///
static std::shared_ptr<PbdObject>
makeTriTissueObj(const std::string& name,
                 const Vec2d& size, const Vec2i& dim, const Vec3d& center,
                 const Quatd& orientation)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> triMesh = makeTriangleGrid(size, dim, center, orientation);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1e-6);
    pbdParams->m_uniformMassValue = 0.00001;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_viscousDampingCoeff = 0.025;

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(triMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));

    // Setup the Object
    tissueObj->setVisualGeometry(triMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(triMesh);
    tissueObj->setCollidingGeometry(triMesh);
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

class PbdObjectCollisionTest : public VisualTestManager
{
public:
    void createScene()
    {
        // Setup the scene
        m_scene = std::make_shared<Scene>("ThinTissueOnPlane");
        m_scene->getActiveCamera()->setPosition(0.0, 0.4, -0.7);
        m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
        m_scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

        ASSERT_NE(m_pbdObj, nullptr) << "Missing a pbdObj for PbdObjectCollisionTest";
        auto pointSet = std::dynamic_pointer_cast<PointSet>(m_pbdObj->getPhysicsGeometry());
        m_currVerticesPtr = pointSet->getVertexPositions();
        m_prevVertices    = *m_currVerticesPtr;
        m_scene->addSceneObject(m_pbdObj);

        ASSERT_NE(m_collidingGeometry, nullptr);
        m_cdObj = std::make_shared<CollidingObject>("obj2");
        m_cdObj->setVisualGeometry(m_collidingGeometry);
        m_cdObj->setCollidingGeometry(m_collidingGeometry);
        m_cdObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
        m_scene->addSceneObject(m_cdObj);

        m_pbdCollision = std::make_shared<PbdObjectCollision>(m_pbdObj, m_cdObj, m_collisionName);
        m_pbdCollision->setFriction(m_friction);
        m_pbdCollision->setRestitution(m_restitution);
        m_scene->addInteraction(m_pbdCollision);

        // Debug geometry to visualize collision data
        m_cdDebugObject = std::make_shared<CollisionDataDebugObject>();
        m_cdDebugObject->setInputCD(m_pbdCollision->getCollisionDetection()->getCollisionData());
        m_cdDebugObject->setPrintContacts(m_printContacts);
        m_scene->addSceneObject(m_cdDebugObject);

        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                m_cdDebugObject->debugUpdate();
                if (m_pauseOnContact && !m_timerPaused
                    && m_cdDebugObject->getInputCD()->elementsA.size() > 0)
                {
                    m_timerPaused = true;
                    m_sceneManager->pause();
                }
            });
        connect<Event>(m_sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                // Run in realtime at a slightly slowed down speed
                // Still fixed, but # of iterations may vary by system
                m_pbdObj->getPbdModel()->getConfig()->m_dt =
                    m_sceneManager->getDt() * 0.5;
            });

        // Assert the vertices stay within bounds and below min displacement
        connect<Event>(m_sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const VecDataArray<double, 3>& vertices = *m_currVerticesPtr;
                ASSERT_TRUE(assertBounds(vertices, Vec3d(-1.0, -1.0, -1.0), Vec3d(1.0, 1.0, 1.0)));
                ASSERT_TRUE(assertMinDisplacement(m_prevVertices, vertices, 0.01));
                m_prevVertices = vertices;
            });

        // Light
        auto light = std::make_shared<DirectionalLight>();
        light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        light->setIntensity(1.0);
        m_scene->addLight("Light", light);
    }

public:
    std::shared_ptr<PbdObject>       m_pbdObj     = nullptr;
    std::shared_ptr<CollidingObject> m_cdObj      = nullptr;
    std::shared_ptr<Geometry> m_collidingGeometry = nullptr;

    std::shared_ptr<PbdObjectCollision> m_pbdCollision = nullptr;
    std::string m_collisionName = "";
    double      m_friction      = 0.0;
    double      m_restitution   = 0.0;
    std::shared_ptr<CollisionDataDebugObject> m_cdDebugObject = nullptr;

    std::shared_ptr<VecDataArray<double, 3>> m_currVerticesPtr;
    VecDataArray<double, 3> m_prevVertices;

    bool m_pauseOnContact = false;
    bool m_printContacts  = false;
};

TEST_F(PbdObjectCollisionTest, ThinTissueOnPlane_ImplicitGeometryToPointSetCD_NoFriction)
{
    // Setup a triangle tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the plane to drop on
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "ImplicitGeometryToPointSetCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    createScene();
    runFor(2.0);
}

TEST_F(PbdObjectCollisionTest, ThinTissueOnPlane_ImplicitGeometryToPointSetCD_Friction)
{
    // Setup a triangle tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the plane to drop on
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(-1.0, 2.0, 0.0);
    implicitGeom->setPosition(0.0, -0.05, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "ImplicitGeometryToPointSetCD";
    m_friction      = 0.2;
    m_restitution   = 0.0;

    createScene();
    runFor(2.0);
}

TEST_F(PbdObjectCollisionTest, ThinTissueOnPlane_PointSetToPlaneCD_NoFriction)
{
    // Setup a triangle tissue
    m_pbdObj = makeTriTissueObj("Tissue",
        Vec2d(0.1, 0.1), Vec2i(3, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))));

    // Setup the plane to drop on
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    createScene();
    runFor(2.0);
}

TEST_F(PbdObjectCollisionTest, TetTissueOnPlane_NoMapping_PointSetToPlaneCD_NoFriction)
{
    // Setup a tet tissue without mapping
    m_pbdObj = makeTetTissueObj("Tissue",
        Vec3d(0.1, 0.05, 0.1), Vec3i(3, 2, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))), true);

    // Setup the plane to drop on
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    createScene();
    runFor(2.0);
}

TEST_F(PbdObjectCollisionTest, TetTissueOnPlane_Mapping_PointSetToPlaneCD_NoFriction)
{
    // Setup a tet tissue with mapping
    m_pbdObj = makeTetTissueObj("Tissue",
        Vec3d(0.1, 0.05, 0.1), Vec3i(3, 2, 3), Vec3d::Zero(),
        Quatd(Rotd(0.4, Vec3d(0.0, 0.0, 1.0))), false);

    // Setup the plane to drop on
    auto implicitGeom = std::make_shared<Plane>();
    implicitGeom->setNormal(0.0, 1.0, 0.0);
    implicitGeom->setPosition(0.0, -0.1, 0.0);
    implicitGeom->setWidth(0.5);
    m_collidingGeometry = implicitGeom;

    m_collisionName = "PointSetToPlaneCD";
    m_friction      = 0.0;
    m_restitution   = 0.0;

    createScene();
    runFor(2.0);
}