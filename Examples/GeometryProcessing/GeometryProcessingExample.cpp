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
#include "imstkDataArray.h"
#include "imstkImageData.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkQuadricDecimate.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief This example demonstrates erosion of a mesh
///
int
main()
{
    Logger::startLogger();

    // simManager and Scene
    imstkNew<Scene> scene("GeometryProcessing");

    auto                         coarseTetMesh  = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    std::shared_ptr<SurfaceMesh> coarseSurfMesh = coarseTetMesh->extractSurfaceMesh();

    // Compute DT
    imstkNew<SurfaceMeshDistanceTransform> createSdf;
    createSdf->setInputMesh(coarseSurfMesh);
    createSdf->setDimensions(50, 50, 50);
    createSdf->update();

    // Erode
    const double       erosionDist = 0.2;
    DataArray<double>& scalars     = *std::dynamic_pointer_cast<DataArray<double>>(createSdf->getOutputImage()->getScalars());
    for (int i = 0; i < scalars.size(); i++)
    {
        scalars[i] += erosionDist;
    }

    // Extract surface
    imstkNew<SurfaceMeshFlyingEdges> isoExtract;
    isoExtract->setInputImage(createSdf->getOutputImage());
    isoExtract->update();

    // Reduce surface
    imstkNew<QuadricDecimate> reduce;
    reduce->setInputMesh(isoExtract->getOutputMesh());
    reduce->setTargetReduction(0.5);
    reduce->update();

    // Create the scene object
    imstkNew<SceneObject> sceneObj("Mesh");
    // Create the eroded visual model
    {
        imstkNew<VisualModel>    surfMeshModel(reduce->getOutput());
        imstkNew<RenderMaterial> material;
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setLineWidth(4.0);
        material->setEdgeColor(Color::Color::Orange);
        surfMeshModel->setRenderMaterial(material);
        sceneObj->addVisualModel(surfMeshModel);
    }
    // Create the original mesh visual model
    {
        imstkNew<VisualModel>    surfMeshModel(coarseSurfMesh);
        imstkNew<RenderMaterial> material;
        material->setColor(Color::Red);
        material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        material->setLineWidth(1.0);
        material->setOpacity(0.2f);
        surfMeshModel->setRenderMaterial(material);
        sceneObj->addVisualModel(surfMeshModel);
    }
    scene->addSceneObject(sceneObj);

    // Set Camera configuration
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 12.0, 12.0));

    // Light
    imstkNew<DirectionalLight> light("light");
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->start();
    }

    return 0;
}
