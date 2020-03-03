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

#include "imstkSimulationManager.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkAPIUtilities.h"

using namespace imstk;

///
/// \brief This example demonstrates the cloth simulation
/// using Position based dynamics
///
int
main()
{
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("PBDCloth");

    // Create surface mesh
    auto             surfMesh = std::make_shared<SurfaceMesh>();
    StdVectorOfVec3d vertList;
    const double     width  = 10.0;
    const double     height = 10.0;
    const int        nRows  = 11;
    const int        nCols  = 11;
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
    surfMesh->setInitialVertexPositions(vertList);
    surfMesh->setVertexPositions(vertList);

    // Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i* nCols + j, (i + 1) * nCols + j, i * nCols + j + 1 } };
            tri[1] = { { (i + 1) * nCols + j + 1, i * nCols + j + 1, (i + 1) * nCols + j } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    surfMesh->setTrianglesVertices(triangles);

    // Create Object & Model
    auto deformableObj = std::make_shared<PbdObject>("Cloth");
    auto pbdModel      = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(surfMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // Constraints
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 0.1);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 0.001);
    std::vector<size_t> fixedNodes(nCols);
    for (size_t i = 0; i < fixedNodes.size(); i++)
    {
        fixedNodes[i] = i;
    }
    pbdParams->m_fixedNodeIds = fixedNodes;

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity          = Vec3d(0, -9.8, 0);
    pbdParams->m_dt               = 0.03;
    pbdParams->m_maxIter          = 5;

    // Set the parameters
    pbdModel->configure(pbdParams);
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->setPhysicsGeometry(surfMesh);

    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::LightGray);
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);
    deformableObj->addVisualModel(surfMeshModel);

    // Solver
    auto pbdSolver = std::make_shared<PbdSolver>();
    pbdSolver->setPbdObject(deformableObj);
    scene->addNonlinearSolver(pbdSolver);

    // Light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);

    // Light (red)
    auto colorLight = std::make_shared<SpotLight>("colorLight");
    colorLight->setPosition(Vec3d(-5, -3, 5));
    colorLight->setFocalPoint(Vec3d(0, -5, 5));
    colorLight->setIntensity(100);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(30);

    // Add in scene
    scene->addLight(whiteLight);
    scene->addLight(colorLight);
    scene->addSceneObject(deformableObj);

    scene->getCamera()->setFocalPoint(0, -5, 5);
    scene->getCamera()->setPosition(-15., -5.0, 15.0);

    // Start
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::PAUSED);

    return 0;
}
