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
#include "imstkDebugGeometryObject.h"
#include "imstkDirectionalLight.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleEmbeddedCH.h"
#include "NeedleInteraction.h"
#include "NeedleObject.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#include "imstkRigidObjectController.h"
#else
#include "imstkMouseDeviceClient.h"
#endif

using namespace imstk;

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
/// \brief Spherically project the texture coordinates
///
static void
setSphereTexCoords(std::shared_ptr<SurfaceMesh> surfMesh, const double uvScale)
{
    Vec3d min, max;
    surfMesh->computeBoundingBox(min, max);
    const Vec3d size   = max - min;
    const Vec3d center = (max + min) * 0.5;

    const double radius = (size * 0.5).norm();

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(surfMesh->getNumVertices());
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        Vec3d vertex = surfMesh->getVertexPosition(i) - center;

        // Compute phi and theta on the sphere
        const double theta = asin(vertex[0] / radius);
        const double phi   = atan2(vertex[1], vertex[2]);
        uvCoords[i] = Vec2f(phi / (PI * 2.0) + 0.5, theta / (PI * 2.0) + 0.5) * uvScale;
    }
    surfMesh->setVertexTCoords("tcoords", uvCoordsPtr);
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
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = makeTetGrid(size, dim, center);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();
    setSphereTexCoords(surfMesh, 6.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    // Actual skin young's modulus, 0.42MPa to 0.85Mpa, as reported in papers
    // Actual skin possion ratio, 0.48, as reported in papers
    pbdParams->m_femParams->m_YoungModulus = 420000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.48;
    // FYI:
    //  - Poisson ratio gives shear to bulk, with 0.5 being complete shear
    //    where everything is like a fluid and can slide past each other. 0.0
    //    gives complete bulk where its rigid
    //  - Youngs modulus then gives the scaling of the above in pressure
    //    (pascals).
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 100.0;
    pbdParams->m_dt = 0.001; // realtime used in update calls later in main
    pbdParams->m_iterations = 5;

    // Due to poor boundary conditions turning off gravity is useful. But then makes
    // your tissue look like it's in space (springy and no resistance). So viscous
    // damping is introduced to approximate these conditions.
    //
    // Ultimately this is a result of not modelling everything around the tissue.
    // and poor/hard to model boundary conditions.
    pbdParams->m_gravity = Vec3d::Zero();
    pbdParams->m_viscousDampingCoeff = 0.03; // Removed from velocity

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
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
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    /* material->setShadingModel(RenderMaterial::ShadingModel::PBR);
     auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
     material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
     auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
     material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
     auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
     material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));
     material->setNormalStrength(0.3);*/
    material->setOpacity(0.5);

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);
    clothObj->addVisualModel(visualModel);

    // Add a visual model to render the normals of the surface
    /*imstkNew<VisualModel> normalsVisualModel(surfMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);
    clothObj->addVisualModel(normalsVisualModel);*/

    // Setup the Object
    clothObj->setPhysicsGeometry(tissueMesh);
    clothObj->setCollidingGeometry(surfMesh);
    clothObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(tissueMesh, surfMesh));
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

static std::shared_ptr<NeedleObject>
makeToolObj()
{
    imstkNew<LineMesh>                toolGeometry;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, -0.05, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.05, 0.0);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeometry->initialize(verticesPtr, indicesPtr);

    auto syringeMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Syringes/Disposable_Syringe.stl");
    syringeMesh->rotate(Vec3d(1.0, 0.0, 0.0), -PI_2, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 4.4, 0.0), Geometry::TransformType::ApplyToData);
    syringeMesh->scale(0.0055, Geometry::TransformType::ApplyToData);
    syringeMesh->translate(Vec3d(0.0, 0.1, 0.0));

    imstkNew<NeedleObject> toolObj("NeedleRbdTool");
    toolObj->setVisualGeometry(syringeMesh);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setPhysicsToVisualMap(std::make_shared<IsometricMap>(toolGeometry, syringeMesh));
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 1.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.1, 0.0);

    return toolObj;
}

///
/// \brief This example demonstrates two-way tissue needle contact with a tetrahedral mesh.
/// Constraints are used at the tetrahedrons faces of intersection
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PBDTissueVolumeNeedleContact");
    scene->getActiveCamera()->setPosition(-0.00149496, 0.0562587, 0.168353);
    scene->getActiveCamera()->setFocalPoint(0.00262407, -0.026582, -0.00463737);
    scene->getActiveCamera()->setViewUp(-0.00218222, 0.901896, -0.431947);

    // Setup a tissue with surface collision geometry
    // 0.1m tissue patch 6x3x6 tet grid
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("PBDTissue",
        Vec3d(0.1, 0.025, 0.1), Vec3i(6, 3, 6), Vec3d(0.0, -0.03, 0.0));
    scene->addSceneObject(tissueObj);

    // Setup a tool for the user to move
    std::shared_ptr<NeedleObject> toolObj = makeToolObj();
    toolObj->setForceThreshold(15.0);
    scene->addSceneObject(toolObj);

    // Setup a debug ghost tool for virtual coupling
    auto ghostToolObj = std::make_shared<SceneObject>("ghostTool");
    {
        auto                  toolMesh = std::dynamic_pointer_cast<SurfaceMesh>(toolObj->getVisualGeometry());
        imstkNew<SurfaceMesh> toolGhostMesh;
        toolGhostMesh->initialize(
            std::make_shared<VecDataArray<double, 3>>(*toolMesh->getVertexPositions(Geometry::DataType::PreTransform)),
            std::make_shared<VecDataArray<int, 3>>(*toolMesh->getTriangleIndices()));
        ghostToolObj->setVisualGeometry(toolGhostMesh);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Orange);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(5.0);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.3);
        ghostToolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);
    }
    scene->addSceneObject(ghostToolObj);

    // Setup a debug polygon soup for debug contact points
    imstkNew<DebugGeometryObject> debugGeomObj;
    debugGeomObj->setLineWidth(0.1);
    scene->addSceneObject(debugGeomObj);

    // This adds both contact and puncture functionality
    auto interaction = std::make_shared<NeedleInteraction>(tissueObj, toolObj);
    scene->addInteraction(interaction);

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        viewer->setDebugAxesLength(0.1, 0.1, 0.1);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001); // 1ms, 1000hz

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        //hapticManager->setSleepDelay(0.01);
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller(toolObj, hapticDeviceClient);
        controller->setTranslationScaling(0.001);
        controller->setLinearKs(5000.0);
        controller->setAngularKs(5000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.05);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addController(controller);

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Update the ghost debug geometry
                std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
                toolGhostMesh->setRotation(controller->getOrientation());
                toolGhostMesh->setTranslation(controller->getPosition());
                toolGhostMesh->updatePostTransformData();
                toolGhostMesh->postModified();

                ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, controller->getDeviceForce().norm() / 15.0));
            });
#else
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const Vec2d mousePos   = viewer->getMouseDevice()->getPos();
                const Vec3d desiredPos = Vec3d(mousePos[0] - 0.5, mousePos[1] - 0.5, 0.0) * 0.1;
                const Quatd desiredOrientation = Quatd(Rotd(0.0, Vec3d(1.0, 0.0, 0.0)));

                Vec3d virtualForce;
                {
                    const Vec3d fS = (desiredPos - toolObj->getRigidBody()->getPosition()) * 1000.0; // Spring force
                    const Vec3d fD = -toolObj->getRigidBody()->getVelocity() * 100.0;                // Spring damping

                    const Quatd dq       = desiredOrientation * toolObj->getRigidBody()->getOrientation().inverse();
                    const Rotd angleAxes = Rotd(dq);
                    const Vec3d tS       = angleAxes.axis() * angleAxes.angle() * 10000000.0;
                    const Vec3d tD       = -toolObj->getRigidBody()->getAngularVelocity() * 1000.0;

                    virtualForce = fS + fD;
                    (*toolObj->getRigidBody()->m_force)  += virtualForce;
                    (*toolObj->getRigidBody()->m_torque) += tS + tD;
                }

                // Update the ghost debug geometry
                std::shared_ptr<Geometry> toolGhostMesh = ghostToolObj->getVisualGeometry();
                toolGhostMesh->setRotation(desiredOrientation);
                toolGhostMesh->setTranslation(desiredPos);
                toolGhostMesh->updatePostTransformData();
                toolGhostMesh->postModified();

                ghostToolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, virtualForce.norm() / 15.0));
        });
#endif

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();

                // Copy debug geometry
                auto needleEmbeddedCH = std::dynamic_pointer_cast<NeedleEmbeddedCH>(interaction->getEmbeddingCH());
                const std::vector<Vec3d>& debugEmbeddingPts  = needleEmbeddedCH->m_debugEmbeddingPoints;
                const std::vector<Vec3i>& debugEmbeddingTris = needleEmbeddedCH->m_debugEmbeddedTriangles;
                debugGeomObj->clear();
                for (size_t i = 0; i < debugEmbeddingPts.size(); i++)
                {
                    debugGeomObj->addPoint(debugEmbeddingPts[i]);
                }
                auto verticesPtr = std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry())->getVertexPositions();
                VecDataArray<double, 3>& vertices = *verticesPtr;
                for (size_t i = 0; i < debugEmbeddingTris.size(); i++)
                {
                    debugGeomObj->addTriangle(
                        vertices[debugEmbeddingTris[i][0]],
                        vertices[debugEmbeddingTris[i][1]],
                        vertices[debugEmbeddingTris[i][2]]);
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