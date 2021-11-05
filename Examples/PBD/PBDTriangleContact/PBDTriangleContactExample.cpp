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
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkVisualModel.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates tissue object
/// \param name
///
static std::shared_ptr<PbdObject>
makePbdTriangle(const std::string& name)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    const double                      triangleScale = 3.0;
    imstkNew<SurfaceMesh>             triangleMesh;
    imstkNew<VecDataArray<double, 3>> verticesPtr(3);
    (*verticesPtr)[0] = Vec3d(-0.5, 0.0, -0.5) * triangleScale;
    (*verticesPtr)[1] = Vec3d(0.0, 0.0, 0.5) * triangleScale;
    (*verticesPtr)[2] = Vec3d(0.5, 0.0, -0.5) * triangleScale;
    imstkNew<VecDataArray<int, 3>> indicesPtr(1);
    (*indicesPtr)[0] = Vec3i(0, 1, 2);
    triangleMesh->initialize(verticesPtr, indicesPtr);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.1;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.2;
    pbdParams->m_iterations = 3;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(triangleMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setOpacity(0.5);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(triangleMesh);
    visualModel->setRenderMaterial(material);
    clothObj->addVisualModel(visualModel);

    imstkNew<VisualModel> normalsVisualModel;
    normalsVisualModel->setGeometry(triangleMesh);
    normalsVisualModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    normalsVisualModel->getRenderMaterial()->setPointSize(0.5);
    clothObj->addVisualModel(normalsVisualModel);

    // Setup the Object
    clothObj->setPhysicsGeometry(triangleMesh);
    clothObj->setCollidingGeometry(triangleMesh);
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

///
/// \brief This example demonstrates the collision with an infinitely
/// stiff triangle, serves as a test case for jitter. The point should
/// be able to rest on the triangle without either moving
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PBDTriangleContact");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup pbd object
    std::shared_ptr<PbdObject>               pbdObject      = makePbdTriangle("PbdTriangle");
    std::shared_ptr<VecDataArray<double, 3>> pbdVerticesPtr =
        std::dynamic_pointer_cast<SurfaceMesh>(pbdObject->getPhysicsGeometry())->getVertexPositions();
    scene->addSceneObject(pbdObject);

    // Setup test point
    imstkNew<PointSet>                pointGeom;
    imstkNew<VecDataArray<double, 3>> verticesPtr(1);
    (*verticesPtr)[0] = ((*pbdVerticesPtr)[0] + (*pbdVerticesPtr)[1] + (*pbdVerticesPtr)[2]) / 3.0;
    pointGeom->initialize(verticesPtr);
    pointGeom->translate(Vec3d(0.0, 2.0, 0.0));

    imstkNew<CollidingObject> pointObject("Point");
    pointObject->setVisualGeometry(pointGeom);
    pointObject->setCollidingGeometry(pointGeom);
    pointObject->getVisualModel(0)->getRenderMaterial()->setRecomputeVertexNormals(false);
    pointObject->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blue);
    pointObject->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Points);
    pointObject->getVisualModel(0)->getRenderMaterial()->setPointSize(10.0);
    scene->addSceneObject(pointObject);

    auto interaction = std::make_shared<PbdObjectCollision>(pbdObject, pointObject);
    scene->getCollisionGraph()->addInteraction(interaction);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);
        std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer())->setAxesLength(0.0, 0.0, 0.0);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

        Vec3d initPos[3] = { (*pbdVerticesPtr)[0], (*pbdVerticesPtr)[1], (*pbdVerticesPtr)[2] };
        connect<Event>(scene, &Scene::configureTaskGraph,
            [&](Event*)
        {
            // Add a manual constraint after the pbd solve
            scene->getTaskGraph()->insertAfter(pbdObject->getPbdModel()->getSolveNode(),
                        std::make_shared<TaskNode>([&]()
            {
                // Constrain the position of the 3 vertices of the triangle
                // too their original locations
                // Completely rigid, if there is jitter it will be very noticable

                // We prented it fully converged as a distance constraint
                (*pbdVerticesPtr)[0] = initPos[0];
                (*pbdVerticesPtr)[1] = initPos[1];
                (*pbdVerticesPtr)[2] = initPos[2];
                }));
            });

        // Move the point
        connect<Event>(sceneManager, SceneManager::preUpdate,
            [&](Event*)
        {
            if (viewer->getKeyboardDevice()->getButton('s') == KEY_PRESS)
            {
                pointGeom->translate(Vec3d(0.0, -0.01, 0.0));
                pointGeom->postModified();
            }
            else if (viewer->getKeyboardDevice()->getButton('w') == KEY_PRESS)
            {
                pointGeom->translate(Vec3d(0.0, 0.01, 0.0));
                pointGeom->postModified();
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