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
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "NeedleInteraction.h"
#include "NeedleObject.h"

#ifdef iMSTK_USE_OPENHAPTICS
#include "imstkHapticDeviceManager.h"
#include "imstkHapticDeviceClient.h"
#else
#include "imstkMouseDeviceClient3D.h"
#endif

using namespace imstk;

///
/// \brief Create pbd string object
///
static std::shared_ptr<PbdObject>
makePbdString(
    const std::string& name,
    const Vec3d& pos, const Vec3d& dir, const int numVerts,
    const double stringLength)
{
    imstkNew<PbdObject> stringObj(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, dir, stringLength, numVerts);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 100.0);
    pbdParams->enableBendConstraint(100000.0, 1);
    pbdParams->enableBendConstraint(100000.0, 2);
    pbdParams->m_fixedNodeIds     = { 0, 1, 19, 20 };
    pbdParams->m_uniformMassValue = 0.002 / numVerts; // grams
    pbdParams->m_gravity = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt      = 0.0005;                    // Overwritten for real time

    // Requires large amounts of iterations the longer, a different
    // solver would help
    pbdParams->m_iterations = 100;
    pbdParams->m_viscousDampingCoeff = 0.01;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(stringMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(6.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setCollidingGeometry(stringMesh);
    stringObj->setDynamicalModel(pbdModel);

    return stringObj;
}

///
/// \brief Generate a static/immovable tissue for static suturing
///
static std::shared_ptr<CollidingObject>
makeTissueObj()
{
    imstkNew<CollidingObject> tissueObj("tissue");

    imstkNew<OrientedBox> box1(Vec3d(0.0, -0.1, -0.1), Vec3d(0.1, 0.025, 0.1));
    imstkNew<VisualModel> box1Model;
    box1Model->setGeometry(box1);
    box1Model->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::Gouraud);
    box1Model->getRenderMaterial()->setColor(Color::LightSkin);
    tissueObj->addVisualModel(box1Model);

    tissueObj->setCollidingGeometry(box1);

    imstkNew<OrientedBox> box2(Vec3d(0.0, -0.105, -0.1), Vec3d(0.1001, 0.025, 0.1001));
    imstkNew<VisualModel> box2Model;
    box2Model->setGeometry(box2);
    box2Model->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::Gouraud);
    box2Model->getRenderMaterial()->setColor(Color::darken(Color::Yellow, 0.2));
    tissueObj->addVisualModel(box2Model);

    return tissueObj;
}

static std::shared_ptr<SceneObject>
makeToolObj(std::string name)
{
    auto surfMesh =
        MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Clamps/Gregory Suture Clamp/gregory_suture_clamp.obj");

    auto toolObj = std::make_shared<SceneObject>(name);
    toolObj->setVisualGeometry(surfMesh);
    auto renderMaterial = std::make_shared<RenderMaterial>();
    renderMaterial->setColor(Color::LightGray);
    renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
    renderMaterial->setRoughness(0.5);
    renderMaterial->setMetalness(1.0);
    toolObj->getVisualModel(0)->setRenderMaterial(renderMaterial);

    return toolObj;
}

///
/// \brief This example is an initial suturing example testbed. It provides the constraint
/// required for an arc shaped needle puncturing vs a static/immovable tissue. What it
/// does not do:
///  - The tissue is not deformable yet, so insertion is a bit stiff
///  - It only constrains the arc to the surface point it punctures not the volume
///  - The suture thread isn't constrained yet
///  - Ability to graps/release the needle, combining into one body
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PBDStaticSuture");

    // Create the arc needle
    imstkNew<NeedleObject> needleObj;
    needleObj->setForceThreshold(2.0);
    scene->addSceneObject(needleObj);

    // Create the suture pbd-based string
    const double               stringLength      = 0.2;
    const int                  stringVertexCount = 30;
    std::shared_ptr<PbdObject> sutureThreadObj   =
        makePbdString("SutureThread", Vec3d(0.0, 0.0, 0.018), Vec3d(0.0, 0.0, 1.0),
            stringVertexCount, stringLength);
    scene->addSceneObject(sutureThreadObj);

    // Create a static box for tissue
    std::shared_ptr<CollidingObject> tissueObj = makeTissueObj();
    scene->addSceneObject(tissueObj);

    // Create clamps that follow the needle around
    std::shared_ptr<SceneObject> clampsObj = makeToolObj("Clamps");
    scene->addSceneObject(clampsObj);

    // Create ghost clamps to show real position of hand under virtual coupling
    std::shared_ptr<SceneObject> ghostClampsObj = makeToolObj("GhostClamps");
    ghostClampsObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Orange);
    scene->addSceneObject(ghostClampsObj);

    // Add point based collision between the tissue & suture thread
    auto interaction = std::make_shared<PbdObjectCollision>(sutureThreadObj, tissueObj, "ImplicitGeometryToPointSetCD");
    interaction->setFriction(0.0);
    scene->addInteraction(interaction);

    // Add needle constraining behaviour between the tissue & arc needle
    auto needleInteraction = std::make_shared<NeedleInteraction>(tissueObj, needleObj);
    scene->addInteraction(needleInteraction);

    // Adjust the camera
    scene->getActiveCamera()->setFocalPoint(0.00138345, -0.0601133, -0.0261938);
    scene->getActiveCamera()->setPosition(0.00137719, 0.0492882, 0.201508);
    scene->getActiveCamera()->setViewUp(-0.000780726, 0.901361, -0.433067);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.01, 0.01, 0.01);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        // Setup a simulation manager to manage renders & scene updates
        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001); // 1ms, 1000hz

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager>       hapticManager;
        std::shared_ptr<HapticDeviceClient> deviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);
        const double translationScaling = 0.001;
        const Vec3d  offset = Vec3d(0.05, -0.05, 0.0);
#else
        imstkNew<MouseDeviceClient3D> deviceClient(viewer->getMouseDevice());
        deviceClient->setOrientation(Quatd(Rotd(1.57, Vec3d(0.0, 1.0, 0.0))));
        const double translationScaling = 0.1;
        const Vec3d  offset = Vec3d(-0.05, -0.1, -0.005);

        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseScroll,
            [&](MouseEvent* e)
            {
                const Quatd delta = Quatd(Rotd(e->m_scrollDx * 0.1, Vec3d(0.0, 0.0, 1.0)));
                deviceClient->setOrientation(deviceClient->getOrientation() * delta);
            });
#endif

        imstkNew<RigidObjectController> controller;
        controller->setControlledObject(needleObj);
        controller->setDevice(deviceClient);
        controller->setTranslationOffset(offset);
        controller->setTranslationScaling(translationScaling);
        controller->setLinearKs(1000.0);
        controller->setAngularKs(10000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.2);
        controller->setSmoothingKernelSize(5);
        controller->setUseForceSmoothening(true);
        scene->addControl(controller);

        // Update the timesteps for real time
        connect<Event>(sceneManager, &SceneManager::preUpdate,
            [&](Event*)
            {
                needleObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
                // sutureThreadObj->getPbdModel()->getConfig()->m_dt  = sceneManager->getDt();
            });
        // Constrain the first two vertices of the string to the needle
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                auto needleLineMesh = std::dynamic_pointer_cast<LineMesh>(needleObj->getPhysicsGeometry());
                auto sutureLineMesh = std::dynamic_pointer_cast<LineMesh>(sutureThreadObj->getPhysicsGeometry());
                (*sutureLineMesh->getVertexPositions())[1] = (*needleLineMesh->getVertexPositions())[0];
                (*sutureLineMesh->getVertexPositions())[0] = (*needleLineMesh->getVertexPositions())[1];
            });
        // Transform the clamps relative to the needle
        const Vec3d clampOffset = Vec3d(-0.009, 0.01, 0.001);
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                clampsObj->getVisualGeometry()->setTransform(
                    needleObj->getVisualGeometry()->getTransform() *
                    mat4dTranslate(clampOffset) *
                    mat4dRotation(Rotd(PI, Vec3d(0.0, 1.0, 0.0))));
                clampsObj->getVisualGeometry()->postModified();
            });
        // Transform the ghost tool clamps to show the real tool location
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                ghostClampsObj->getVisualGeometry()->setTransform(
                    mat4dTranslate(controller->getPosition()) * mat4dRotation(controller->getOrientation()) *
                    mat4dTranslate(clampOffset) *
                    mat4dRotation(Rotd(PI, Vec3d(0.0, 1.0, 0.0))));
                ghostClampsObj->getVisualGeometry()->updatePostTransformData();
                ghostClampsObj->getVisualGeometry()->postModified();
                ghostClampsObj->getVisualModel(0)->getRenderMaterial()->setOpacity(std::min(1.0, controller->getDeviceForce().norm() / 5.0));
            });

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}