/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkCollisionUtils.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

static void
setFabricTextures(std::shared_ptr<RenderMaterial> material)
{
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fabricDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fabricNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fabricORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));
}

static void
setFleshTextures(std::shared_ptr<RenderMaterial> material)
{
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));
}

///
/// \brief Creates cloth object
/// \param name
/// \param cloth width
/// \param cloth height
/// \param cloth row count
/// \param cloth column count
///
static std::shared_ptr<PbdObject>
makeClothObj(const std::string& name,
             const Vec2d        size,
             const Vec2i        dim,
             const Vec3d        pos)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(pos, size, dim,
            Quatd::Identity(), 2.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 0.05);
    pbdParams->m_uniformMassValue = size[0] * size[1] / (dim[0] * dim[1]) * 0.01;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 10;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    setFleshTextures(material);
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setCollidingGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

///
/// \brief This example demonstrates the cloth simulation
/// using Position based dynamics
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    imstkNew<Scene>            scene("PBDCloth");
    std::shared_ptr<PbdObject> clothObj =
        makeClothObj("Cloth", Vec2d(5.0, 5.0), Vec2i(4, 4), Vec3d(0.0, 6.0, 0.0));
    scene->addSceneObject(clothObj);

    auto            planeObj =  std::make_shared<CollidingObject>("Plane");
    imstkNew<Plane> plane(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    plane->setWidth(10.0);
    planeObj->setVisualGeometry(plane);
    planeObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    planeObj->setCollidingGeometry(plane);
    scene->addSceneObject(planeObj);

    // Adjust camera
    scene->getActiveCamera()->setFocalPoint(-0.116722, 1.70485, 0.625839);
    scene->getActiveCamera()->setPosition(2.25549, 8.07292, 14.8692);

    auto clothCollision = std::make_shared<PbdObjectCollision>(clothObj, planeObj, "PointSetToPlaneCD");
    scene->addInteraction(clothCollision);

    auto pbdGrasping = std::make_shared<PbdObjectGrasping>(clothObj);
    scene->addInteraction(pbdGrasping);

    // Make two sphere's for indication
    imstkNew<SceneObject> clickObj("clickObj");
    imstkNew<Sphere>      clickSphere(Vec3d(0.0, 0.0, 0.0), 0.1);
    clickObj->setVisualGeometry(clickSphere);
    clickObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::None);
    clickObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Red);
    scene->addSceneObject(clickObj);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

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

        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent* e)
            {
                if (e->m_buttonId == 0)
                {
                    // Get mouse position (0, 1) with origin at bot left of screen
                    const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                    // To NDC coordinates
                    const Vec3d rayDir = scene->getActiveCamera()->getEyeRayDir(
                        Vec2d(mousePos[0] * 2.0 - 1.0, mousePos[1] * 2.0 - 1.0));
                    const Vec3d rayStart = scene->getActiveCamera()->getPosition();

                    pbdGrasping->beginRayPointGrasp(clickSphere, rayStart, rayDir);
                }
            });
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseMove,
            [&](MouseEvent* e)
            {
                // Get mouse position (0, 1) with origin at bot left of screen
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                // To NDC coordinates
                const Vec3d rayDir = scene->getActiveCamera()->getEyeRayDir(
                    Vec2d(mousePos[0] * 2.0 - 1.0, mousePos[1] * 2.0 - 1.0));
                const Vec3d rayStart = scene->getActiveCamera()->getPosition();

                Vec3d iPt = Vec3d::Zero();
                if (CollisionUtils::testRayToPlane(rayStart, rayDir,
                    plane->getPosition(), plane->getNormal(), iPt))
                {
                    clickSphere->setPosition(iPt);
                }
            });
        // Unselect/drop the sphere
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](MouseEvent* e)
            {
                if (e->m_buttonId == 0)
                {
                    pbdGrasping->endGrasp();
                }
            });

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Run the model in real time
                clothObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        driver->start();
    }

    return 0;
}