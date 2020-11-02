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
#include "imstkCDObjectFactory.h"
#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkCollisionGraph.h"
#include "imstkCollisionPair.h"
#include "imstkCollisionDetection.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkLaparoscopicToolController.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

#include "imstkLogger.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"

#include "imstkSphere.h"
#include "imstkCapsule.h"
#include "imstkSurfaceMesh.h"

#include "imstkPBDPickingCH.h"
#include "imstkPbdObjectPickingPair.h"

#include "imstkIsometricMap.h"

// global variables
const std::string phantomOmni1Name = "Default Device";

using namespace imstk;

// Parameters to play with
const double width  = 50.0;
const double height = 50.0;
const int    nRows  = 16;
const int    nCols  = 16;

///
/// \brief Creates cloth geometry
///
static std::shared_ptr<SurfaceMesh>
makeClothGeometry(const double width,
                  const double height,
                  const int    nRows,
                  const int    nCols)
{
    imstkNew<SurfaceMesh> clothMesh;

    StdVectorOfVec3d vertList;
    vertList.resize(nRows * nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertList[i * nCols + j] = Vec3d((double)dx * i, 1.0, (double)dy * j);
        }
    }
    clothMesh->setInitialVertexPositions(vertList);
    clothMesh->setVertexPositions(vertList);

    // Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            const size_t               index1 = i * nCols + j;
            const size_t               index2 = index1 + nCols;
            const size_t               index3 = index1 + 1;
            const size_t               index4 = index2 + 1;

            // Interleave [/][\]
            if (i % 2 ^ j % 2)
            {
                tri[0] = { { index1, index2, index3 } };
                tri[1] = { { index4, index3, index2 } };
            }
            else
            {
                tri[0] = { { index2, index4, index1 } };
                tri[1] = { { index4, index3, index1 } };
            }
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    clothMesh->setTrianglesVertices(triangles);

    return clothMesh;
}

///
/// \brief Creates cloth object
///
static std::shared_ptr<PbdObject>
makeClothObj(const std::string& name,
             const double       width,
             const double       height,
             const int          nRows,
             const int          nCols)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh(makeClothGeometry(width, height, nRows, nCols));

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 1.0e1);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / ((double)nRows * (double)nCols);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_defaultDt  = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

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
/// \brief This example demonstrates the concept of PBD picking
/// for haptic interaction. NOTE: Requires GeoMagic Touch device
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    imstkNew<Scene> scene("PBDPicking");

    // Create the virtual coupling object controller

    // Device Server
    imstkNew<HapticDeviceManager>       server;
    std::shared_ptr<HapticDeviceClient> client = server->makeDeviceClient(phantomOmni1Name);

    // Load the meshes
    auto    upperSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/upper.obj");
    auto    lowerSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/lower.obj");
    auto    pivotSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/laptool/pivot.obj");

    // Object
    imstkNew<Capsule>          geomShaft;
    geomShaft->setLength(20.0);
    geomShaft->setPosition(Vec3d(0.0, 0.0, 10.0));
    geomShaft->setRadius(1.0);
    geomShaft->setOrientationAxis(Vec3d(0.0, 0.0, 1.0));
    imstkNew<CollidingObject> objShaft("ShaftObject");
    objShaft->setVisualGeometry(pivotSurfMesh);
    objShaft->setCollidingGeometry(geomShaft);
    objShaft->setCollidingToVisualMap(std::make_shared<IsometricMap>(geomShaft, pivotSurfMesh));
    scene->addSceneObject(objShaft);

    imstkNew<Sphere>          geomUpperJaw(Vec3d(0.0, 0.0, -25.0), 2.0);
    //geomUpperJaw->setTranslation(Vec3d(0.0, 0.0, -25.0));
    //geomUpperJaw->setOrientationAxis(Vec3d(1.0, 0.0, 0.0));
    
    imstkNew<CollidingObject> objUpperJaw("UpperJawObject");
    objUpperJaw->setVisualGeometry(geomUpperJaw);
    objUpperJaw->setCollidingGeometry(geomUpperJaw);
    //auto mapUpperJaw = std::make_shared<IsometricMap>(geomUpperJaw, upperSurfMesh);
    //objUpperJaw->setCollidingToVisualMap(mapUpperJaw);
    scene->addSceneObject(objUpperJaw);
    
    imstkNew<Sphere>          geomLowerJaw(Vec3d(0.0, 0.0, -25.0), 2.0);
    imstkNew<CollidingObject> objLowerJaw("LowerJawObject");
    objLowerJaw->setVisualGeometry(lowerSurfMesh);
    objLowerJaw->setCollidingGeometry(geomLowerJaw);
    auto mapLowerJaw = std::make_shared<IsometricMap>(geomLowerJaw, lowerSurfMesh);
    objLowerJaw->setCollidingToVisualMap(mapLowerJaw);
    scene->addSceneObject(objLowerJaw);

    std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", width, height, nRows, nCols);
    scene->addSceneObject(clothObj);

    // Create and add virtual coupling object controller in the scene
    imstkNew<LaparoscopicToolController> controller(objShaft, objUpperJaw, objLowerJaw, client);
    scene->addController(controller);

    // Add interaction pair for pbd picking
    imstkNew<PbdObjectPickingPair> pair(clothObj, objUpperJaw, CollisionDetection::Type::PointSetToSphere);
    scene->getCollisionGraph()->addInteraction(pair);

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(200, 200, 200));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 0));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight(light);

    //Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        viewer->addChildThread(server);

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setViewer(viewer);
            viewer->addControl(keyControl);
        }
        // Not perfectly thread safe movement lambda, ijkl movement instead of wasd because d is already used
        std::shared_ptr<KeyboardDeviceClient> keyDevice = viewer->getKeyboardDevice();
        connect<Event>(sceneManager, EventType::PreUpdate, [&](Event*)
        {
            std::shared_ptr<PBDPickingCH> ch = std::static_pointer_cast<PBDPickingCH>(pair->getCollisionHandlingA());
            // Activate picking
            
            if (keyDevice->getButton('i') == KEY_PRESS)
            {
                ch->activatePickConstraints();
            }
            // Unpick
            if (keyDevice->getButton('u') == KEY_PRESS)
            {
                ch->removePickConstraints();
            }
        });

        // Start viewer running, scene as paused
        sceneManager->requestStatus(ThreadStatus::Paused);
        viewer->start();
    }

    return 0;
}
