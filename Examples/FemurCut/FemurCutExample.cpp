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
#include "imstkCollisionGraph.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkImageData.h"
#include "imstkIsometricMap.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLevelSetCH.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkLight.h"
#include "imstkLocalMarchingCubes.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidBodyCH.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkRigidObjectLevelSetCollisionPair.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVTKViewer.h"

#include <vtkObject.h>

using namespace imstk;
using namespace imstk::expiremental;

///
/// \brief Creates a level set obj (poly rendering)
///
std::shared_ptr<LevelSetDeformableObject>
makeLevelsetObj(const std::string& name, std::shared_ptr<LocalMarchingCubes> isoExtract, std::unordered_set<int>& chunksGenerated)
{
    imstkNew<LevelSetDeformableObject> levelsetObj(name);

    std::shared_ptr<ImageData> initLvlSetImage = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/legs/femurBoneSolid_SDF.nii")->cast(IMSTK_DOUBLE);
    const Vec3d&               currSpacing     = initLvlSetImage->getSpacing();

    // Note: Anistropic scaling would invalidate the SDF
    initLvlSetImage->setOrigin(Vec3d(0.0, 0.8, 1.5));

    // Setup the Parameters
    imstkNew<LevelSetModelConfig> lvlSetConfig;
    lvlSetConfig->m_sparseUpdate = true;
    lvlSetConfig->m_substeps     = 30;

    // Too many chunks and you'll hit memory constraints quickly
    // Too little chunks and the updates for a chunk will take too long
    // The chunks must divide the image dimensions (image dim-1 must be divisible by # chunks)
    isoExtract->setInputImage(initLvlSetImage);
    isoExtract->setIsoValue(0.0);
    isoExtract->setNumberOfChunks(Vec3i(32, 9, 9));
    isoExtract->update();

    srand(time(NULL));

    const Vec3i& numChunks = isoExtract->getNumberOfChunks();
    for (int i = 0; i < numChunks[0] * numChunks[1] * numChunks[2]; i++)
    {
        auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(isoExtract->getOutput(i));
        if (surfMesh->getNumVertices() > 0 && chunksGenerated.count(i) == 0)
        {
            imstkNew<VisualModel>    surfMeshModel(isoExtract->getOutput(i));
            imstkNew<RenderMaterial> material;
            material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
            material->setLineWidth(4.0);
            const double r = (rand() % 500) / 500.0;
            const double g = (rand() % 500) / 500.0;
            const double b = (rand() % 500) / 500.0;
            material->setColor(Color(r, g, b));
            material->setEdgeColor(Color::Color::Orange);
            //material->setOpacity(0.7);
            surfMeshModel->setRenderMaterial(material);
            levelsetObj->addVisualModel(surfMeshModel);
            chunksGenerated.insert(i);
        }
    }

    // Setup the Object
    imstkNew<SignedDistanceField> sdf(initLvlSetImage);

    // Setup the Model
    imstkNew<LevelSetModel> model;
    model->setModelGeometry(sdf);
    model->configure(lvlSetConfig);

    levelsetObj->setPhysicsGeometry(sdf);
    levelsetObj->setCollidingGeometry(sdf);
    levelsetObj->setDynamicalModel(model);

    return levelsetObj;
}

std::shared_ptr<RigidObject2>
makeRigidObj(const std::string& name)
{
    imstkNew<RigidBodyModel2> rbdModel;
    rbdModel->getConfig()->m_maxNumIterations       = 8;
    rbdModel->getConfig()->m_velocityDamping        = 1.0;
    rbdModel->getConfig()->m_angularVelocityDamping = 1.0;
    rbdModel->getConfig()->m_maxNumConstraints      = 20;

    // Create the first rbd, plane floor
    imstkNew<RigidObject2> rigidObj("Tool");

    {
        auto toolMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scalpel/Scalpel_Hull_Subdivided_Shifted.stl");
        toolMesh->rotate(Vec3d(0.0, 1.0, 0.0), 3.14, Geometry::TransformType::ApplyToData);
        toolMesh->rotate(Vec3d(1.0, 0.0, 0.0), -1.57, Geometry::TransformType::ApplyToData);
        toolMesh->scale(Vec3d(0.07, 0.07, 0.07), Geometry::TransformType::ApplyToData);

        imstkNew<RenderMaterial> toolMaterial;
        toolMaterial->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        toolMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
        toolMaterial->setMetalness(0.9f);
        toolMaterial->setRoughness(0.4f);
        toolMaterial->setDiffuseColor(Color(0.7, 0.7, 0.7));

        // Create the object
        rigidObj->setVisualGeometry(toolMesh);
        rigidObj->setPhysicsGeometry(toolMesh);
        rigidObj->setCollidingGeometry(toolMesh);
        rigidObj->setDynamicalModel(rbdModel);
        rigidObj->getRigidBody()->m_mass = 1.0;
        rigidObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
        rigidObj->getRigidBody()->m_initPos = Vec3d(0.0, 1.0, 2.0);
    }
    return rigidObj;
}

///
/// \brief This example demonstrates cutting a femur bone with a tool
/// Some of the example parameters may need to be tweaked for differing
/// systems
///
int
main()
{
    vtkObject::GlobalWarningDisplayOff();

    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("FemurCut");

    std::shared_ptr<LocalMarchingCubes>       isoExtract = std::make_shared<LocalMarchingCubes>();
    std::unordered_set<int>                   chunksGenerated; // Lazy generation of chunks
    std::shared_ptr<LevelSetDeformableObject> lvlSetObj = makeLevelsetObj("LevelSetObj", isoExtract, chunksGenerated);
    scene->addSceneObject(lvlSetObj);

    std::shared_ptr<RigidObject2> rbdObj = makeRigidObj("RigidObj");
    scene->addSceneObject(rbdObj);

    imstkNew<SceneObject> rbdGhostObj("RigidObjGhost");
    imstkNew<SurfaceMesh> ghostMesh;
    ghostMesh->deepCopy(std::dynamic_pointer_cast<SurfaceMesh>(rbdObj->getPhysicsGeometry()));
    rbdGhostObj->setVisualGeometry(ghostMesh);
    std::shared_ptr<RenderMaterial> ghostMat = std::make_shared<RenderMaterial>(*rbdObj->getVisualModel(0)->getRenderMaterial());
    ghostMat->setOpacity(0.4);
    rbdGhostObj->getVisualModel(0)->setRenderMaterial(ghostMat);
    scene->addSceneObject(rbdGhostObj);

    imstkNew<RigidObjectLevelSetCollisionPair> interaction(rbdObj, lvlSetObj);
    {
        auto colHandlerA = std::dynamic_pointer_cast<RigidBodyCH>(interaction->getCollisionHandlingA());
        colHandlerA->setUseFriction(false);
        colHandlerA->setStiffness(0.05); // inelastic collision

        auto colHandlerB = std::dynamic_pointer_cast<LevelSetCH>(interaction->getCollisionHandlingB());
        colHandlerB->setLevelSetVelocityScaling(0.05);
        colHandlerB->setKernel(3, 1.0);
        //colHandlerB->setLevelSetVelocityScaling(0.0); // Can't push the levelset
        colHandlerB->setUseProportionalVelocity(true);
    }
    std::shared_ptr<CollisionDetection> cd = interaction->getCollisionDetection();

    scene->getCollisionGraph()->addInteraction(interaction);

    // Light (white)
    imstkNew<DirectionalLight> whiteLight("whiteLight");
    whiteLight->setDirection(Vec3d(0.0, -8.0, 5.0));
    whiteLight->setIntensity(1.0);
    scene->addLight(whiteLight);

    // Adjust camera
    scene->getActiveCamera()->setFocalPoint(0.25, 0.83, 1.58);
    scene->getActiveCamera()->setPosition(0.243, 1.06, 1.95);
    scene->getActiveCamera()->setViewUp(0.05, 0.86, -0.51);

    {
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Add a module to run the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);

        imstkNew<HapticDeviceManager>       hapticManager;
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();

        imstkNew<RigidObjectController> controller(rbdObj, hapticDeviceClient);
        {
            controller->setLinearKd(1000.0 * 0.9);
            controller->setLinearKs(100000.0 * 0.9);
            controller->setAngularKs(300000000.0);
            controller->setAngularKd(400000.0);
            controller->setForceScaling(0.001);

            // The particular device we are using doesn't produce this quantity
            // with this flag its computed
            // in code
            controller->setComputeVelocity(true);
            controller->setComputeAngularVelocity(true);

            controller->setTranslationScaling(0.0015);
            controller->setTranslationOffset(Vec3d(0.1, 0.9, 1.6));
            controller->setSmoothingKernelSize(30);

            scene->addController(controller);
        }

        connect<Event>(sceneManager->getActiveScene(), &Scene::configureTaskGraph, [&](Event*)
        {
            std::shared_ptr<TaskGraph> taskGraph = sceneManager->getActiveScene()->getTaskGraph();

            // Pipe the changes from the levelset into local marching cubes
            // Compute this before the levelset is evolved
            taskGraph->insertBefore(lvlSetObj->getLevelSetModel()->getQuantityEvolveNode(0),
                    std::make_shared<TaskNode>([&]()
            {
                for (auto i : lvlSetObj->getLevelSetModel()->getNodesToUpdate())
                {
                    isoExtract->setModified(std::get<0>(i.second));
                }
            }, "Isosurface: SetModifiedVoxels"));
        });
        connect<Event>(viewer, &Viewer::preUpdate, [&](Event*)
        {
            // Update any chunks that contain a voxel which was set modified
            isoExtract->update();

            // Create meshes for chunks if they now contain vertices (and weren't already generated)
            // You could just create all the chunks, but this saves some memory for internal/empty ones
            const Vec3i& numChunks = isoExtract->getNumberOfChunks();
            for (int i = 0; i < numChunks[0] * numChunks[1] * numChunks[2]; i++)
            {
                auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(isoExtract->getOutput(i));
                if (surfMesh->getNumVertices() > 0 && chunksGenerated.count(i) == 0)
                {
                    imstkNew<VisualModel> surfMeshModel(isoExtract->getOutput(i));
                    imstkNew<RenderMaterial> material;
                    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
                    material->setLineWidth(4.0);
                    const double r = (rand() % 500) / 500.0;
                    const double g = (rand() % 500) / 500.0;
                    const double b = (rand() % 500) / 500.0;
                    material->setColor(Color(r, g, b));
                    material->setEdgeColor(Color::Color::Orange);
                    //material->setOpacity(0.7);
                    surfMeshModel->setRenderMaterial(material);
                    lvlSetObj->addVisualModel(surfMeshModel);
                    chunksGenerated.insert(i);
                }
            }
        });
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            rbdObj->getRigidBodyModel2()->getConfig()->m_dt  = sceneManager->getDt();
            lvlSetObj->getLevelSetModel()->getConfig()->m_dt = sceneManager->getDt();

            // Also apply controller transform to ghost geometry
            ghostMesh->setTranslation(controller->getPosition());
            ghostMesh->setRotation(controller->getRotation());
            ghostMesh->updatePostTransformData();
            ghostMesh->postModified();
        });

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->addModule(hapticManager);
        driver->setDesiredDt(0.001); // Little over 1000ups

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