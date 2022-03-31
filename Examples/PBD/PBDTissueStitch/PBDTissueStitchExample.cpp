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
#include "imstkCapsule.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMouseSceneControl.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectStitching.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

#define USE_FEM
//#define USE_HAPTICS

#ifdef USE_HAPTICS
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#else
#include "imstkDummyClient.h"
#include "imstkMouseDeviceClient.h"
#endif

///
/// \brief Creates a tetraheral grid
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of grid
///
static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
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

    auto                    uvCoordsPtr = std::make_shared<VecDataArray<float, 2>>(dim[0] * dim[1] * dim[2]);
    VecDataArray<float, 2>& uvCoords    = *uvCoordsPtr;
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                uvCoords[x + dim[0] * (y + dim[1] * z)] = Vec2f(static_cast<float>(x) / dim[0], static_cast<float>(z) / dim[2]) * 3.0;
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
    tetMesh->setVertexTCoords("uvs", uvCoordsPtr);
    return tetMesh;
}

///
/// \brief Creates triangle grid geometry
/// \param cloth width (x), height (z)
/// \param cloth dimensions/divisions
///
static std::shared_ptr<SurfaceMesh>
makeTriangleGrid(const Vec2d  size,
                 const Vec2i  dim,
                 const Vec3d  center,
                 const double uvScale)
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

    auto                    uvCoordsPtr = std::make_shared<VecDataArray<float, 2>>(dim[0] * dim[1]);
    VecDataArray<float, 2>& uvCoords    = *uvCoordsPtr;
    iter = 0;
    for (int y = 0; y < dim[1]; y++)
    {
        for (int x = 0; x < dim[0]; x++, iter++)
        {
            uvCoords[iter] = Vec2f(static_cast<float>(x) / dim[0], static_cast<float>(y) / dim[1]) * uvScale;
        }
    }

    auto triMesh = std::make_shared<SurfaceMesh>();
    triMesh->initialize(verticesPtr, indicesPtr);
    triMesh->setVertexTCoords("uvs", uvCoordsPtr);
    return triMesh;
}

///
/// \brief Creates tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTetTissueObj(const std::string& name,
                 const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = makeTetGrid(size, dim, center);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
#ifdef USE_FEM
    // Use FEMTet constraints (42k - 85k for tissue, but we want
    // something much more stretchy to wrap)
    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4; // 0.48 for tissue
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
#else
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 0.01);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.4);
#endif
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.00001;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_viscousDampingCoeff = 0.05;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(tissueMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(0.5);

    // Setup the Object
    tissueObj->setVisualGeometry(surfMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

static std::shared_ptr<PbdObject>
makeTriTissueObj(const std::string& name,
                 const Vec2d& size, const Vec2i& dim, const Vec3d& center)
{
    auto tissueObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> triMesh = makeTriangleGrid(size, dim, center, 1.0);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PbdModelConfig>();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 0.1);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1e-6);
    pbdParams->m_uniformMassValue = 0.00001;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_viscousDampingCoeff = 0.025;

    // Fix the borders
    for (int y = 0; y < dim[1]; y++)
    {
        for (int x = 0; x < dim[0]; x++)
        {
            if (x == 0)
            {
                pbdParams->m_fixedNodeIds.push_back(x + dim[0] * y);
            }
        }
    }

    // Setup the Model
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(triMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    material->setOpacity(0.5);

    // Setup the Object
    tissueObj->setVisualGeometry(triMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(triMesh);
    tissueObj->setCollidingGeometry(triMesh);
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

static std::shared_ptr<RigidObject2>
makeToolObj()
{
    auto toolGeom    = std::make_shared<LineMesh>();
    auto verticesPtr = std::make_shared<VecDataArray<double, 3>>(2);
    (*verticesPtr)[0] = Vec3d(0.0, -0.05, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.05, 0.0);
    auto indicesPtr = std::make_shared<VecDataArray<int, 2>>(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeom->initialize(verticesPtr, indicesPtr);

    auto toolObj = std::make_shared<RigidObject2>("ToolObj");
    toolObj->setVisualGeometry(toolGeom);
    toolObj->setCollidingGeometry(toolGeom);
    toolObj->setPhysicsGeometry(toolGeom);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    auto rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 0.3;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);

    return toolObj;
}

///
/// \brief This example demonstrates stitching interaction with pbd tissues
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    const double capsuleRadius = 0.02;
    const bool   useThinTissue = false;
    const double tissueLength  = 0.15;

    // Setup the scene
    auto scene = std::make_shared<Scene>("PbdTissueStitch");
    scene->getActiveCamera()->setPosition(0.0012, 0.0451, 0.1651);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tet tissue
    std::shared_ptr<PbdObject> tissueObj = nullptr;
    if (useThinTissue)
    {
        tissueObj = makeTriTissueObj("Tissue",
            Vec2d(tissueLength, 0.07), Vec2i(15, 5),
            Vec3d(tissueLength * 0.5, -0.01 - capsuleRadius, 0.0));
    }
    else
    {
        tissueObj = makeTetTissueObj("Tissue",
            Vec3d(tissueLength, 0.01, 0.07), Vec3i(15, 2, 5),
            Vec3d(tissueLength * 0.5, -0.01 - capsuleRadius, 0.0));
    }
    scene->addSceneObject(tissueObj);

    // Setup a capsule to wrap around
    auto cdObj       = std::make_shared<CollidingObject>("Bone");
    auto capsuleGeom = std::make_shared<Capsule>();
    capsuleGeom->setPosition(0.0, 0.0, 0.0);
    capsuleGeom->setRadius(capsuleRadius);
    capsuleGeom->setLength(0.08);
    capsuleGeom->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    cdObj->setVisualGeometry(capsuleGeom);
    cdObj->getVisualModel(0)->getRenderMaterial()->setColor(
        Color(246.0 / 255.0, 127.0 / 255.0, 123.0 / 255.0));
    cdObj->setCollidingGeometry(capsuleGeom);
    scene->addSceneObject(cdObj);

    std::shared_ptr<RigidObject2> toolObj = makeToolObj();
    scene->addSceneObject(toolObj);

    // Setup CD with a cylinder CD object
    auto col1 = std::make_shared<PbdObjectCollision>(tissueObj, cdObj, "SurfaceMeshToCapsuleCD");
    col1->setFriction(0.0);
    scene->addInteraction(col1);

    auto stitching = std::make_shared<PbdObjectStitching>(tissueObj);
    scene->addInteraction(stitching);

    // Lights
    auto light1 = std::make_shared<DirectionalLight>();
    light1->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light1->setIntensity(0.5);
    scene->addLight("light1", light1);
    auto light2 = std::make_shared<DirectionalLight>();
    light2->setFocalPoint(Vec3d(-5.0, -8.0, -5.0));
    light2->setIntensity(0.5);
    scene->addLight("light2", light2);

    // Run the simulation
    {
        // Setup a viewer to render
        auto viewer = std::make_shared<VTKViewer>();
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.001, 0.001, 0.001);

        // Setup a scene manager to advance the scene
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

#ifdef USE_HAPTICS
        auto hapticManager = std::make_shared<HapticDeviceManager>();
        hapticManager->setSleepDelay(0.1); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> deviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);
#else
        auto deviceClient = std::make_shared<DummyClient>();
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const Vec2d& pos = viewer->getMouseDevice()->getPos();
                if (useThinTissue)
                {
                    deviceClient->setPosition(Vec3d(40.0, 40.0, -(pos[1] * 100.0 - 50.0)));
                    deviceClient->setOrientation(Quatd(Rotd(-0.6, Vec3d(0.0, 0.0, 1.0))));
                }
                else
                {
                    deviceClient->setPosition(Vec3d(37.0, 0.0, -(pos[1] * 100.0 - 50.0)));
                    deviceClient->setOrientation(Quatd(Rotd(0.65, Vec3d(0.0, 0.0, 1.0))));
                }
            });
#endif

        auto controller = std::make_shared<RigidObjectController>(toolObj, deviceClient);
        controller->setTranslationScaling(0.001);
        controller->setLinearKs(1000.0);
        controller->setAngularKs(10000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.0045);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addController(controller);

#ifdef USE_HAPTICS
        connect<ButtonEvent>(deviceClient, &HapticDeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    auto toolGeom   = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());
                    const Vec3d& v1 = toolGeom->getVertexPosition(0);
                    const Vec3d& v2 = toolGeom->getVertexPosition(1);
                    stitching->beginRayPointStitch(v1, (v2 - v1).normalized());
                }
            });
#endif

        double t = 0.0;
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                // Toggle gravity
                if (e->m_key == 'g')
                {
                    Vec3d& g = tissueObj->getPbdModel()->getConfig()->m_gravity;
                    g = Vec3d(0.0, -static_cast<double>(!(g.norm() > 0.0)), 0.0);
                }
                // Perform stitch
                else if (e->m_key == 's')
                {
                    auto toolGeom   = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());
                    const Vec3d& v1 = toolGeom->getVertexPosition(0);
                    const Vec3d& v2 = toolGeom->getVertexPosition(1);
                    stitching->beginRayPointStitch(v1, (v2 - v1).normalized());
                }
                // Reset
                else if (e->m_key == 'r')
                {
                    t = 0.0;
                }
            });

        // Record the intial positions
        std::vector<Vec3d> initPositions;

        auto pointMesh =
            std::dynamic_pointer_cast<PointSet>(tissueObj->getPhysicsGeometry());
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointMesh->getVertexPositions();
        VecDataArray<double, 3>&                 vertices    = *verticesPtr;
        const std::vector<size_t>                fixedNodes  = tissueObj->getPbdModel()->getConfig()->m_fixedNodeIds;
        for (size_t i = 0; i < fixedNodes.size(); i++)
        {
            initPositions.push_back(vertices[fixedNodes[i]]);
        }
        bool stopped = false;

        // Script the movement of the tissues fixed points
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const double dt = sceneManager->getDt();
                t += dt;
                if (t < 10.5)
                {
                    for (size_t i = 0; i < fixedNodes.size(); i++)
                    {
                        Vec3d initPos = initPositions[i];
                        Vec3d& pos    = vertices[fixedNodes[i]];

                        const double r = (capsuleGeom->getPosition().head<2>() - initPos.head<2>()).norm();
                        pos = Vec3d(-sin(t) * r, -cos(t) * r, initPos[2]);
                    }
                }
                else
                {
                    if (!stopped)
                    {
                        stopped = true;
                        tissueObj->getPbdModel()->getConfig()->m_fixedNodeIds.clear();

                        // Clear and reinit all constraints
                        tissueObj->initialize();
                    }
                }
            });

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}