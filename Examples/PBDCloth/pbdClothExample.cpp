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
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkAPIUtilities.h"

using namespace imstk;

void pbdCloth()
{
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("PBDCloth");

    // a. Construct a sample triangular mesh

    // b. Add nodal data
    auto surfMesh = std::make_shared<SurfaceMesh>();
    StdVectorOfVec3d vertList;
    const double width = 10.0;
    const double height = 10.0;
    const int nRows = 11;
    const int nCols = 11;
    vertList.resize(nRows*nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertList[i*nCols + j] = Vec3d((double)dx*i, 1.0, (double)dy*j);
        }
    }
    surfMesh->setInitialVertexPositions(vertList);
    surfMesh->setVertexPositions(vertList);

    // c. Add connectivity data
    std::vector<SurfaceMesh::TriangleArray> triangles;
    for (std::size_t i = 0; i < nRows - 1; ++i)
    {
        for (std::size_t j = 0; j < nCols - 1; j++)
        {
            SurfaceMesh::TriangleArray tri[2];
            tri[0] = { { i*nCols + j, (i + 1)*nCols + j, i*nCols + j + 1 } };
            tri[1] = { { (i + 1)*nCols + j + 1, i*nCols + j + 1, (i + 1)*nCols + j } };
            triangles.push_back(tri[0]);
            triangles.push_back(tri[1]);
        }
    }

    surfMesh->setTrianglesVertices(triangles);

    // Object & Model
    auto deformableObj = std::make_shared<PbdObject>("Cloth");
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(surfMesh);
    pbdModel->configure(/*Number of constraints*/ 2,
        /*Constraint configuration*/ "Distance 0.1",
        /*Constraint configuration*/ "Dihedral 0.001",
        /*Mass*/ 1.0,
        /*Gravity*/ "0 -9.8 0",
        /*TimeStep*/ 0.03,
        /*FixedPoint*/ "1 2 3 4 5 6 7 8 9 10 11",
        /*NumberOfIterationInConstraintSolver*/ 5);
    deformableObj->setDynamicalModel(pbdModel);
    deformableObj->setVisualGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(surfMesh);

    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setDiffuseColor(Color::LightGray);
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    surfMesh->setRenderMaterial(material);

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

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    scene->getCamera()->setFocalPoint(0, -5, 5);
    scene->getCamera()->setPosition(-15., -5.0, 15.0);

    // Start
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);
}

int main()
{
    pbdCloth();

    return 0;
}
