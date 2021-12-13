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
#include "imstkCollisionHandling.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOrientedBox.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPlane.h"
#include "imstkPointSetToCapsuleCD.h"
#include "imstkPointSetToOrientedBoxCD.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshToCapsuleCD.h"
#include "imstkSurfaceMeshToSphereCD.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates cloth geometry
/// \param cloth width
/// \param cloth height
/// \param cloth row count
/// \param cloth column count
///
static std::shared_ptr<SurfaceMesh>
makeClothGeometry(const double width,
                  const double height,
                  const int    nRows,
                  const int    nCols,
                  const double uvScale)
{
    imstkNew<SurfaceMesh> clothMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(nRows * nCols);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const double                      dy       = width / (nCols - 1);
    const double                      dx       = height / (nRows - 1);
    const Vec3d                       halfSize = Vec3d(height, 0.0, width) * 0.5;
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d(dx * static_cast<double>(i), 0.05, dy * static_cast<double>(j) - 1.0) - halfSize;
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> indicesPtr;
    VecDataArray<int, 3>&          indices = *indicesPtr.get();
    for (int i = 0; i < nRows - 1; i++)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            const int index1 = i * nCols + j;
            const int index2 = index1 + nCols;
            const int index3 = index1 + 1;
            const int index4 = index2 + 1;

            // Interleave [/][\]
            if (i % 2 ^ j % 2)
            {
                indices.push_back(Vec3i(index2, index1, index3));
                indices.push_back(Vec3i(index3, index4, index2));
            }
            else
            {
                indices.push_back(Vec3i(index4, index2, index1));
                indices.push_back(Vec3i(index3, index4, index1));
            }
        }
    }

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(nRows * nCols);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            uvCoords[i * nCols + j] = Vec2f(static_cast<float>(i) / nRows, static_cast<float>(j) / nCols) * uvScale;
        }
    }

    clothMesh->initialize(verticesPtr, indicesPtr);
    clothMesh->setVertexTCoords("uvs", uvCoordsPtr);

    return clothMesh;
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
             const double       width,
             const double       height,
             const int          rowCount,
             const int          colCount)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh = makeClothGeometry(width, height, rowCount, colCount, 2.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0e1);
    pbdParams->m_uniformMassValue = width * height / (rowCount * colCount);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color::Blue);
    imstkNew<VisualModel> visualModel(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setCollidingGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

///
/// \brief This example demonstrates pointset collision with cloth for multiple primitives
/// Press 1, 2, 3, & 4 to cycle through primitive shapes
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Capsule>     capsule(Vec3d(0.0, -4.0, 0.0), 2.0, 5.0, Quatd(Rotd(PI_2, Vec3d(0.0, 0.0, 1.0))));
    imstkNew<Sphere>      sphere(Vec3d(0.0, -2.0, 0.0), 2.0);
    imstkNew<OrientedBox> cube(Vec3d(0.0, -4.0, 0.0), Vec3d(2.5, 2.5, 2.5));
    imstkNew<Plane>       plane(Vec3d(0.0, -2.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    plane->setWidth(20.0);
    std::shared_ptr<Geometry> geometries[] = { capsule, sphere, cube, plane };

    // Setup a scene
    imstkNew<Scene>                     scene("PBDClothCollision");
    std::shared_ptr<PbdObject>          clothObj       = nullptr;
    std::shared_ptr<CollidingObject>    collisionObj   = nullptr;
    std::shared_ptr<PbdObjectCollision> pbdInteraction = nullptr;
    {
        clothObj = makeClothObj("Cloth", 10.0, 10.0, 16, 16);
        scene->addSceneObject(clothObj);

        collisionObj = std::make_shared<CollidingObject>("CollidingObject");
        collisionObj->setCollidingGeometry(capsule);
        for (int i = 0; i < 4; i++)
        {
            auto visualModel = std::make_shared<VisualModel>(geometries[i]);
            visualModel->getRenderMaterial()->setBackFaceCulling(false);
            visualModel->getRenderMaterial()->setOpacity(0.5);
            visualModel->hide();
            collisionObj->addVisualModel(visualModel);
        }
        collisionObj->getVisualModel(0)->show();
        scene->addSceneObject(collisionObj);

        // Collision
        pbdInteraction = std::make_shared<PbdObjectCollision>(clothObj, collisionObj, "PointSetToCapsuleCD");
        pbdInteraction->setFriction(0.4);
        pbdInteraction->setRestitution(0.0); // Inelastic collision
        scene->addInteraction(pbdInteraction);

        // Adjust camera
        scene->getActiveCamera()->setFocalPoint(0.0, -2.0, 0.0);
        scene->getActiveCamera()->setPosition(5.0, 4.0, 18.0);
    }

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
        driver->setDesiredDt(0.001);

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

        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
        {
            int indexToShow = -1;
            std::shared_ptr<CollisionDetectionAlgorithm> newCDMethod = nullptr;
            // Switch to sphere and reset
            if (e->m_key == '1')
            {
                indexToShow = 0;
                newCDMethod = std::make_shared<PointSetToCapsuleCD>();
            }
            // Switch to capsule and reset
            else if (e->m_key == '2')
            {
                indexToShow = 1;
                newCDMethod = std::make_shared<PointSetToSphereCD>();
            }
            // Switch to cube and reset
            else if (e->m_key == '3')
            {
                indexToShow = 2;
                newCDMethod = std::make_shared<PointSetToOrientedBoxCD>();
            }
            // Switch to plane and reset
            else if (e->m_key == '4')
            {
                indexToShow = 3;
                newCDMethod = std::make_shared<PointSetToPlaneCD>();
            }
            // Switch to sphere vs surface and reset
            else if (e->m_key == '5')
            {
                indexToShow = 1;
                newCDMethod = std::make_shared<SurfaceMeshToSphereCD>();
            }
            // Switch to capsule vs surface and reset
            else if (e->m_key == '6')
            {
                indexToShow = 0;
                newCDMethod = std::make_shared<SurfaceMeshToCapsuleCD>();
            }

            if (indexToShow != -1)
            {
                std::shared_ptr<Geometry> newCDGeometry =
                    collisionObj->getVisualModel(indexToShow)->getGeometry();
                for (int i = 0; i < 4; i++)
                {
                    collisionObj->getVisualModel(i)->hide();
                }
                collisionObj->getVisualModel(indexToShow)->show();
                collisionObj->setCollidingGeometry(newCDGeometry);

                newCDMethod->setInputGeometryA(clothObj->getCollidingGeometry());
                newCDMethod->setInputGeometryB(newCDGeometry);
                pbdInteraction->setCollisionDetection(newCDMethod);
                pbdInteraction->getCollisionHandlingA()->setInputCollisionData(newCDMethod->getCollisionData());

                scene->buildTaskGraph();
                scene->initTaskGraph();
                scene->reset();
            }
             });

        driver->start();
    }

    return 0;
}
