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
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
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
    imstkNew<SurfaceMesh> clothMesh;
    StdVectorOfVec3d      vertList;

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
    pbdParams->m_defaultDt  = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setColor(Color::LightGray);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    auto clothVisualModel = std::make_shared<VisualModel>(clothMesh);
    clothVisualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(clothVisualModel);
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

    // Light (white)
    imstkNew<DirectionalLight> whiteLight("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);

    // Light (red)
    imstkNew<SpotLight> colorLight("colorLight");
    colorLight->setPosition(Vec3d(-5, -3, 5));
    colorLight->setFocalPoint(Vec3d(0, -5, 5));
    colorLight->setIntensity(100);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(30);
    scene->addLight(colorLight);

    // Adjust camera
    scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 5.0);
    scene->getActiveCamera()->setPosition(-15.0, -5.0, 15.0);

    // Adds a custom physics step to print out maximum velocity
    std::shared_ptr<PbdModel> pbdModel = clothObj->getPbdModel();
    connect<Event>(scene, EventType::Configure,
        [&](Event*)
        {
            // Get the graph
            std::shared_ptr<TaskGraph> graph = scene->getTaskGraph();

            // First write the graph before we make modifications, just to show the changes
            imstkNew<TaskGraphVizWriter> writer;
            writer->setInput(graph);
            writer->setFileName("taskGraphConfigureExampleOld.svg");
            writer->write();

            imstkNew<TaskNode> printMaxVelocity([&]()
                {
                    const StdVectorOfVec3d& velocities = *pbdModel->getCurrentState()->getVelocities().get();
                    double maxVel = std::numeric_limits<double>::min();
                    for (size_t i = 0; i < velocities.size(); i++)
                    {
                        const double vel = velocities[i].squaredNorm();
                        if (vel > maxVel)
                        {
                            maxVel = vel;
                        }
                    }
                    LOG(INFO) << "Max Velocity: " << std::sqrt(maxVel);
                }, "PrintMaxVelocity");

            // After IntegratePosition
            graph->insertAfter(pbdModel->getIntegratePositionNode(), printMaxVelocity);

            // Write the modified graph
            writer->setFileName("taskGraphConfigureExampleNew.svg");
            writer->write();
        });

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        viewer->start();
    }

    return 0;
}
