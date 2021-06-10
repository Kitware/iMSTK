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
#include "imstkColorFunction.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTaskGraphVizWriter.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

static std::shared_ptr<SurfaceMesh>
makeClothGeometry(
    const double width, const double height, const int nRows, const int nCols)
{
    // Create surface mesh
    imstkNew<SurfaceMesh>             clothMesh;
    imstkNew<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();

    vertices.resize(nRows * nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d((double)dx * i, 1.0, (double)dy * j);
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> trianglesPtr;
    VecDataArray<int, 3>&          triangles = *trianglesPtr.get();
    for (int i = 0; i < nRows - 1; ++i)
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
                triangles.push_back(Vec3i(index1, index2, index3));
                triangles.push_back(Vec3i(index4, index3, index2));
            }
            else
            {
                triangles.push_back(Vec3i(index2, index4, index1));
                triangles.push_back(Vec3i(index4, index3, index1));
            }
        }
    }
    clothMesh->initialize(verticesPtr, trianglesPtr);

    return clothMesh;
}

static std::shared_ptr<PbdObject>
makeClothObj(const std::string& name, double width, double height, int nRows, int nCols)
{
    auto clothObj = std::make_shared<PbdObject>(name);

    std::shared_ptr<SurfaceMesh> clothMesh = makeClothGeometry(width, height, nRows, nCols);

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1e2);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 1e1);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / (nRows * nCols);
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup visual models
    imstkNew<VisualModel> clothModel;
    clothModel->setGeometry(clothMesh);
    clothModel->getRenderMaterial()->setBackFaceCulling(false);
    clothModel->getRenderMaterial()->setColor(Color::LightGray);
    clothModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    imstkNew<VisualModel> clothSurfaceNormals;
    clothSurfaceNormals->setGeometry(clothMesh);
    clothSurfaceNormals->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    clothSurfaceNormals->getRenderMaterial()->setPointSize(0.5);

    // Setup the Object
    clothObj->addVisualModel(clothModel);
    clothObj->addVisualModel(clothSurfaceNormals);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

///
/// \brief This example demonstrates how to modify the task graph
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PBDCloth");

    const double               width    = 10.0;
    const double               height   = 10.0;
    const int                  nRows    = 16;
    const int                  nCols    = 16;
    std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", width, height, nRows, nCols);
    scene->addSceneObject(clothObj);

    // Adjust camera
    scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 5.0);
    scene->getActiveCamera()->setPosition(-15.0, -5.0, 15.0);

    // Setup some scalars
    auto clothGeometry = std::dynamic_pointer_cast<SurfaceMesh>(clothObj->getPhysicsGeometry());
    auto scalarsPtr    = std::make_shared<DataArray<double>>(clothGeometry->getNumVertices());
    std::fill_n(scalarsPtr->getPointer(), scalarsPtr->size(), 0.0);
    clothGeometry->setVertexScalars("scalars", scalarsPtr);

    // Setup the material for the scalars
    std::shared_ptr<RenderMaterial> material = clothObj->getVisualModel(0)->getRenderMaterial();
    material->setScalarVisibility(true);
    std::shared_ptr<ColorFunction> colorFunc = std::make_shared<ColorFunction>();
    colorFunc->setNumberOfColors(2);
    colorFunc->setColor(0, Color::Green);
    colorFunc->setColor(1, Color::Red);
    colorFunc->setColorSpace(ColorFunction::ColorSpace::RGB);
    colorFunc->setRange(0.0, 2.0);
    material->setColorLookupTable(colorFunc);

    // Adds a custom physics step to print out maximum velocity
    connect<Event>(scene, &Scene::configureTaskGraph,
        [&](Event*)
    {
        // Get the graph
        std::shared_ptr<TaskGraph> graph = scene->getTaskGraph();

        // First write the graph before we make modifications, just to show the changes
        imstkNew<TaskGraphVizWriter> writer;
        writer->setInput(graph);
        writer->setFileName("taskGraphConfigureExampleOld.svg");
        writer->write();

        // This node computes displacements and sets the color to the magnitude
        std::shared_ptr<TaskNode> computeVelocityScalars = std::make_shared<TaskNode>([&]()
        {
            /*const StdVectorOfVec3d& initPos = clothGeometry->getInitialVertexPositions();
            const StdVectorOfVec3d& currPos = clothGeometry->getVertexPositions();
            StdVectorOfReal& scalars = *scalarsPtr;
            for (size_t i = 0; i < initPos.size(); i++)
            {
                scalars[i] = (currPos[i] - initPos[i]).norm();
            }*/
            const VecDataArray<double, 3>& velocities =
                *std::dynamic_pointer_cast<VecDataArray<double, 3>>(clothGeometry->getVertexAttribute("Velocities"));
            DataArray<double>& scalars = *scalarsPtr;
            for (int i = 0; i < velocities.size(); i++)
            {
                scalars[i] = velocities[i].norm();
            }
        }, "ComputeVelocityScalars");

        // After IntegratePosition
        graph->insertAfter(clothObj->getUpdateGeometryNode(), computeVelocityScalars);

        // Write the modified graph
        writer->setFileName("taskGraphConfigureExampleNew.svg");
        writer->write();
    });

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

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

    // Write out simulation geometry
    MeshIO::write(clothGeometry, "cloth.vtk");

    return 0;
}
