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
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkObjectInteractionFactory.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

// Enable this macro to generate many dragons
#define BIG_SCENE

using namespace imstk;

///
/// \brief Generate a random color
///
Color getRandomColor();

void generateDragon(const std::shared_ptr<imstk::Scene>& scene,
                    const Vec3d&                         translation,
                    std::shared_ptr<SurfaceMesh>&        surfMesh,
                    std::shared_ptr<PbdObject>&          deformableObj);

///
/// \brief Create a surface mesh
/// \param nRows number of vertices in x-direction
/// \param nCols number of vertices in y-direction
///
std::shared_ptr<SurfaceMesh> createUniformSurfaceMesh(const double width, const double height, const int nRows, const int nCols);

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamics
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("PbdCollision");

    std::shared_ptr<SurfaceMesh> floorMesh = createUniformSurfaceMesh(100.0, 100.0, 2, 2);

    imstkNew<RenderMaterial> floorMaterial;
    floorMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    imstkNew<VisualModel> floorMeshModel(floorMesh);
    floorMeshModel->setRenderMaterial(floorMaterial);

    imstkNew<PbdObject> floorObj("Floor");
    floorObj->setCollidingGeometry(floorMesh);
    floorObj->setVisualGeometry(floorMesh);
    floorObj->setPhysicsGeometry(floorMesh);

    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(floorMesh);

    // configure model
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->m_uniformMassValue = 0.0;
    pbdParams->collisionParams->m_proximity = 0.1;
    pbdParams->m_iterations = 0;

    // Set the parameters
    pbdModel->configure(pbdParams);
    floorObj->setDynamicalModel(pbdModel);
    scene->addSceneObject(floorObj);

#ifdef BIG_SCENE
    const int expandsXZ = 1;
    const int expandsY  = 2;
#endif
    const double shiftX     = 5.0;
    const double distanceXZ = 10.0;
    const double distanceY  = 5.0;
    const double minHeight  = -5.0;

    std::vector<std::shared_ptr<PbdObject>> pbdObjs;

#ifdef BIG_SCENE
    for (int i = -expandsXZ; i < expandsXZ; ++i)
    {
        for (int j = 0; j < expandsY; ++j)
        {
            for (int k = -expandsXZ; k < expandsXZ; ++k)
#else
    int i = 0;
    {
        int j = 0;
        {
            int k = 0;
#endif
            {
                std::shared_ptr<SurfaceMesh> mesh;
                std::shared_ptr<PbdObject>   pbdObj;
                Vec3d                        translation(shiftX + i * distanceXZ, minHeight + j * distanceY, k * distanceXZ);
                generateDragon(scene, translation, mesh, pbdObj);
                pbdObjs.push_back(pbdObj);

                scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(pbdObj, floorObj,
                    InteractionType::PbdObjToPbdObjCollision, CollisionDetection::Type::SurfaceMeshToSurfaceMesh));
            }
        }
    }

    for (size_t i = 0; i < pbdObjs.size(); ++i)
    {
        for (size_t j = i + 1; j < pbdObjs.size(); ++j)
        {
            scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(pbdObjs[i], pbdObjs[j],
                InteractionType::PbdObjToPbdObjCollision, CollisionDetection::Type::SurfaceMeshToSurfaceMesh));
        }
    }

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Set Camera configuration
    std::shared_ptr<Camera> cam = scene->getActiveCamera();
    cam->setPosition(Vec3d(0, 15, 30));
    cam->setFocalPoint(Vec3d(0, 0, 0));

    {
        // Add a module to run the viewer
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);
        viewer->setWindowTitle("PbdCollision");
        viewer->setSize(1920, 1080);

        // Add a module to run the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

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

Color
getRandomColor()
{
    Color color(0.0, 0.0, 0.0, 1.0);
    while (true)
    {
        for (unsigned int i = 0; i < 3; ++i)
        {
            color.rgba[i] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        }
        if (color.rgba[0] > 0.95
            || color.rgba[1] > 0.95
            || color.rgba[2] > 0.95)
        {
            break;
        }
    }
    return color;
}

void
generateDragon(const std::shared_ptr<imstk::Scene>& scene,
               const Vec3d&                         translation,
               std::shared_ptr<SurfaceMesh>&        surfMesh,
               std::shared_ptr<PbdObject>&          deformableObj)
{
    // Load a sample mesh
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    LOG_IF(FATAL, (!tetMesh)) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";

    // Rotate a rando angle
    tetMesh->rotate(Vec3d(0, 1, 0), static_cast<double>(rand()), Geometry::TransformType::ApplyToData);

    // Translate
    tetMesh->translate(translation, Geometry::TransformType::ApplyToData);

    // Trick to force update geometry postUpdateTransform
    const auto positions = tetMesh->getVertexPositions();
    (void)positions;

    surfMesh = tetMesh->extractSurfaceMesh();

    imstkNew<RenderMaterial> material;
#if 0
    // Wireframe color
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    material->setColor(getRandomColor()); // Wireframe color
    material->setLineWidth(2);
#else
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setColor(getRandomColor());
#endif

    imstkNew<VisualModel> surfMeshModel(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    static int count = -1;
    ++count;
    deformableObj = std::make_shared<PbdObject>("Dragon-" + std::to_string(count));
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(tetMesh);
    deformableObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(tetMesh, surfMesh));

    // Create model and object
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tetMesh);

    // configure model
    imstkNew<PBDModelConfig> pbdParams;

    // FEM constraint
    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);

    // Other parameters
    pbdParams->m_uniformMassValue = 5.0;
    pbdParams->m_gravity    = Vec3d(0, -1.0, 0);
    pbdParams->m_defaultDt  = 0.01;
    pbdParams->m_iterations = 20;
    pbdParams->collisionParams->m_proximity = 0.5;

    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);

    scene->addSceneObject(deformableObj);
}

std::shared_ptr<SurfaceMesh>
createUniformSurfaceMesh(const double width, const double height, const int nRows, const int nCols)
{
    // Build floor geometry
    const double dy = width / static_cast<double>(nCols - 1);
    const double dx = height / static_cast<double>(nRows - 1);

    imstkNew<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    vertices.resize(static_cast<int>(nRows * nCols));
    for (int i = 0; i < nRows; i++)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = static_cast<double>(dy * j);
            const double x = static_cast<double>(dx * i);
            vertices[i * nCols + j] = Vec3d(x - height * 0.5, -10.0, y - width * 0.5);
        }
    }

    // c. Add connectivity data
    imstkNew<VecDataArray<int, 3>> trianglesPtr;
    VecDataArray<int, 3>&          triangles = *trianglesPtr.get();
    for (int i = 0; i < nRows - 1; i++)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            triangles.push_back(Vec3i(i * nCols + j, i * nCols + j + 1, (i + 1) * nCols + j));
            triangles.push_back(Vec3i((i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1));
        }
    }

    imstkNew<SurfaceMesh> surfMesh;
    surfMesh->initialize(verticesPtr, trianglesPtr);

    return surfMesh;
}
