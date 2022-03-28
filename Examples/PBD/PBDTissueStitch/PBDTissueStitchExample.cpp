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
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkRigidObjectController.h"
#include "imstkRigidObject2.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRbdConstraint.h"
#include "imstkPbdObjectStitching.h"

using namespace imstk;

#define USE_FEM

///
/// \brief Creates a tetraheral grid
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of grid
///
static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<TetrahedralMesh> tissueMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const Vec3d                       dx       = size.cwiseQuotient((dim - Vec3i(1, 1, 1)).cast<double>());
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                vertices[x + dim[0] * (y + dim[1] * z)] = Vec3i(x, y, z).cast<double>().cwiseProduct(dx) - size * 0.5 + center;
            }
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 4>> indicesPtr;
    VecDataArray<int, 4>&          indices = *indicesPtr.get();
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

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
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

    tissueMesh->initialize(verticesPtr, indicesPtr);
    tissueMesh->setVertexTCoords("uvs", uvCoordsPtr);

    return tissueMesh;
}

///
/// \brief Creates tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<PbdObject> tissueObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = makeTetGrid(size, dim, center);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
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
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);;
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    //pbdParams->m_contactStiffness = 0.4;
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
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tissueMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    imstkNew<RenderMaterial> material;
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

static std::shared_ptr<RigidObject2>
makeToolObj()
{
    imstkNew<LineMesh> toolGeom;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, -0.05, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.05, 0.0);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeom->initialize(verticesPtr, indicesPtr);

    imstkNew<RigidObject2> toolObj("ToolObj");
    toolObj->setVisualGeometry(toolGeom);
    toolObj->setCollidingGeometry(toolGeom);
    toolObj->setPhysicsGeometry(toolGeom);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 0.3;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);

    return toolObj;
}

///
/// \brief This example demonstrates collision interaction with a 3d pbd
/// simulated tissue (tetrahedral)
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    const double capsuleRadius = 0.02;

    // Setup the scene
    imstkNew<Scene> scene("PbdTissueStitch");
    scene->getActiveCamera()->setPosition(0.0012, 0.0451, 0.1651);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tet tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue",
        Vec3d(0.2, 0.01, 0.07), Vec3i(20, 2, 5), Vec3d(0.1, -0.01 - capsuleRadius, 0.0));
    scene->addSceneObject(tissueObj);

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
    auto interaction = std::make_shared<PbdObjectCollision>(tissueObj, cdObj, "SurfaceMeshToCapsuleCD");
    interaction->setFriction(0.0);
    interaction->setRestitution(0.0);
    scene->addInteraction(interaction);

   /* auto grasping = std::make_shared<PbdObjectGrasping>(tissueObj);
    scene->addInteraction(grasping);*/

    auto stitching = std::make_shared<PbdObjectStitching>(tissueObj);
    scene->addInteraction(stitching);

    // Light
    imstkNew<DirectionalLight> light1;
    light1->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light1->setIntensity(0.5);
    scene->addLight("Light1", light1);

    // Light
    imstkNew<DirectionalLight> light2;
    light2->setFocalPoint(Vec3d(-5.0, -8.0, -5.0));
    light2->setIntensity(0.5);
    scene->addLight("Light2", light2);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.001, 0.001, 0.001);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        hapticManager->setSleepDelay(0.1); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller(toolObj, hapticDeviceClient);
        controller->setTranslationScaling(0.001);
        controller->setLinearKs(1000.0);
        controller->setAngularKs(10000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.0045);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addController(controller);

        connect<ButtonEvent>(hapticDeviceClient, &HapticDeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    auto toolGeom = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());
                    const Vec3d& v1 = toolGeom->getVertexPosition(0);
                    const Vec3d& v2 = toolGeom->getVertexPosition(1);
                    stitching->beginRayPointStitch(v1, (v2 - v1).normalized());
                }
            });
#else
#endif

        // Toggle gravity, perform stitch, & reset
        double t = 0.0;
        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    Vec3d& g = tissueObj->getPbdModel()->getConfig()->m_gravity;
                    if (g.norm() > 0.0)
                    {
                        tissueObj->getPbdModel()->getConfig()->m_gravity = Vec3d(0.0, 0.0, 0.0);
                    }
                    else
                    {
                        tissueObj->getPbdModel()->getConfig()->m_gravity = Vec3d(0.0, -1.0, 0.0);
                    }
                }
                else if (e->m_key == 's')
                {
                    auto toolGeom = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());
                    const Vec3d& v1 = toolGeom->getVertexPosition(0);
                    const Vec3d& v2 = toolGeom->getVertexPosition(1);
                    stitching->beginRayPointStitch(v1, (v2 - v1).normalized());
                }
                else if (e->m_key == 'r')
                {
                    t = 0.0;
                }
            });

        // Record the intial positions
        std::vector<Vec3d> initPositions;

        auto tetMesh =
            std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry());
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = tetMesh->getVertexPositions();
        VecDataArray<double, 3>& vertices = *verticesPtr;
        const std::vector<size_t>& fixedNodes = tissueObj->getPbdModel()->getConfig()->m_fixedNodeIds;
        for (size_t i = 0; i < fixedNodes.size(); i++)
        {
            initPositions.push_back(vertices[fixedNodes[i]]);
        }

        // Script the movement of the tissues fixed points
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                const double dt = sceneManager->getDt();
                t += dt;
                if (t < 9.0)
                {
                    for (size_t i = 0; i < fixedNodes.size(); i++)
                    {
                        Vec3d initPos = initPositions[i];
                        Vec3d& pos = vertices[fixedNodes[i]];

                        const double r = (capsuleGeom->getPosition().head<2>() - initPos.head<2>()).norm();
                        pos = Vec3d(-sin(t) * r, -cos(t) * r, initPos[2]);
                    }
                }
                else if ( t > 11.0)
                {
                    for (size_t i = 0; i < fixedNodes.size(); i++)
                    {
                        tissueObj->getPbdModel()->setPointUnfixed(fixedNodes[i]);
                    }
                }
            });

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}