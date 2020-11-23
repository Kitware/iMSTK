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
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraTriangleMap.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

// mesh file names
const std::string surfMeshFileName = iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj";
const std::string tetMeshFileName  = iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg";

// parameters to play with
const double youngModulus     = 1000.0;
const double poissonRatio     = 0.3;
const double timeStep         = 0.01;
const double contactStiffness = 0.1;
const int    maxIter = 5;

///
/// \brief Create a surface mesh
/// \param nRows number of vertices in x-direction
/// \param nCols number of vertices in y-direction
///
std::shared_ptr<SurfaceMesh> createUniformSurfaceMesh(const double width, const double height, const size_t nRows, const size_t nCols);

///
/// \brief This example demonstrates the collision interaction
/// using Position based dynamics
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PbdCollisionOneDragon");
    {
        scene->getActiveCamera()->setPosition(0, 3.0, 20.0);
        scene->getActiveCamera()->setFocalPoint(0.0, -10.0, 0.0);

        // set up the meshes
        auto                  highResSurfMesh = MeshIO::read<SurfaceMesh>(surfMeshFileName);
        auto                  coarseTetMesh   = MeshIO::read<TetrahedralMesh>(tetMeshFileName);
        imstkNew<SurfaceMesh> coarseSurfMesh;
        coarseTetMesh->extractSurfaceMesh(coarseSurfMesh, true);

        // set up visual model based on high res mesh
        imstkNew<RenderMaterial> material;
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setLineWidth(0.5);
        material->setEdgeColor(Color::Blue);
        material->setShadingModel(RenderMaterial::ShadingModel::Phong);
        imstkNew<VisualModel> surfMeshModel(highResSurfMesh);
        surfMeshModel->setRenderMaterial(material);

        // configure the deformable object
        imstkNew<PbdObject> deformableObj("DeformableObj");
        deformableObj->addVisualModel(surfMeshModel);
        deformableObj->setCollidingGeometry(coarseSurfMesh);
        deformableObj->setPhysicsGeometry(coarseTetMesh);
        deformableObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(coarseTetMesh, coarseSurfMesh));
        deformableObj->setPhysicsToVisualMap(std::make_shared<TetraTriangleMap>(coarseTetMesh, highResSurfMesh));

        // Create model and object
        imstkNew<PbdModel> pbdModel;
        pbdModel->setModelGeometry(coarseTetMesh);

        // configure model
        imstkNew<PBDModelConfig> pbdParams;

        // FEM constraint
        pbdParams->m_femParams->m_YoungModulus = youngModulus;
        pbdParams->m_femParams->m_PoissonRatio = poissonRatio;
        pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet,
            PbdFEMConstraint::MaterialType::Corotation);

        // Other parameters
        // \todo use lumped mass
        pbdParams->m_uniformMassValue = 1.0;
        pbdParams->m_gravity    = Vec3d(0, -10.0, 0);
        pbdParams->m_defaultDt  = timeStep;
        pbdParams->m_iterations = maxIter;
        pbdParams->collisionParams->m_proximity = 0.3;
        pbdParams->collisionParams->m_stiffness = 0.1;

        pbdModel->configure(pbdParams);
        deformableObj->setDynamicalModel(pbdModel);

        scene->addSceneObject(deformableObj);

        // Build floor geometry
        auto floorMesh = createUniformSurfaceMesh(100.0, 100.0, 2, 2);

        imstkNew<RenderMaterial> floorMaterial;
        floorMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        imstkNew<VisualModel> floorVisualModel(floorMesh);
        floorVisualModel->setRenderMaterial(floorMaterial);

        imstkNew<PbdObject> floorObj("Floor");
        floorObj->setCollidingGeometry(floorMesh);
        floorObj->setPhysicsGeometry(floorMesh);
        floorObj->addVisualModel(floorVisualModel);

        imstkNew<PbdModel> floorPbdModel;
        floorPbdModel->setModelGeometry(floorMesh);

        // configure model
        imstkNew<PBDModelConfig> floorPbdParams;
        floorPbdParams->m_uniformMassValue = 0.0;
        floorPbdParams->m_iterations       = 0;
        floorPbdParams->collisionParams->m_proximity = -0.1;

        // Set the parameters
        floorPbdModel->configure(floorPbdParams);
        floorObj->setDynamicalModel(floorPbdModel);

        scene->addSceneObject(floorObj);

        // Collision
        scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(deformableObj, floorObj,
            InteractionType::PbdObjToPbdObjCollision, CollisionDetection::Type::MeshToMeshBruteForce));

        // Light
        imstkNew<DirectionalLight> light("Light");
        light->setFocalPoint(Vec3d(5, -8, -5));
        light->setIntensity(1);
        scene->addLight(light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

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

        // Start viewer running, scene as paused
        sceneManager->requestStatus(ThreadStatus::Paused);
        viewer->start();
    }

    return 0;
}

std::shared_ptr<SurfaceMesh>
createUniformSurfaceMesh(const double width, const double height, const size_t nRows, const size_t nCols)
{
    const double dy = width / static_cast<double>(nCols - 1);
    const double dx = height / static_cast<double>(nRows - 1);

    imstkNew<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    vertices.resize(nRows * nCols);

    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; j++)
        {
            const double y = static_cast<double>(dy * j);
            const double x = static_cast<double>(dx * i);
            vertices[i * nCols + j] = Vec3d(x - height * 0.5, -10.0, y - width * 0.5);
        }
    }

    // c. Add connectivity data
    imstkNew<VecDataArray<int, 3>> trianglesPtr;
    VecDataArray<int, 3>&          triangles = *trianglesPtr.get();
    for (int i = 0; i < nRows - 1; ++i)
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
