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
#include "imstkLight.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTaskGraphVizWriter.h"
#include "imstkVisualModel.h"

using namespace imstk;
using namespace imstk::expiremental;

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
    imstkNew<SimulationManager> simManager;
    auto                        scene = simManager->createNewScene("PBDCloth");

    const double               width    = 10.0;
    const double               height   = 10.0;
    const int                  nRows    = 16;
    const int                  nCols    = 16;
    std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", width, height, nRows, nCols);
    scene->addSceneObject(clothObj);

    // Adjust camera
    scene->getCamera()->setFocalPoint(0, -5, 5);
    scene->getCamera()->setPosition(-15., -5.0, 15.0);

    {
        // Setup some scalars
        std::shared_ptr<SurfaceMesh>     clothGeometry = std::dynamic_pointer_cast<SurfaceMesh>(clothObj->getPhysicsGeometry());
        std::shared_ptr<StdVectorOfReal> scalarsPtr    = std::make_shared<StdVectorOfReal>(clothGeometry->getNumVertices());
        std::fill_n(scalarsPtr->data(), scalarsPtr->size(), 0.0);
        clothGeometry->setScalars(scalarsPtr);

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

        std::shared_ptr<PbdModel> pbdModel = clothObj->getPbdModel();
        scene->setTaskGraphConfigureCallback([&](Scene* scene)
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
                const StdVectorOfVec3d& velocities = *pbdModel->getCurrentState()->getVelocities();
                StdVectorOfReal& scalars = *scalarsPtr;
                for (size_t i = 0; i < velocities.size(); i++)
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
    }

    // Start
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Paused);

    return 0;
}
