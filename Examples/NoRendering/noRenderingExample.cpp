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

// vtk includes
#include "vtkMathUtilities.h"

#include <chrono>
#include <thread>

using namespace imstk;

///
/// \brief This example demonstrates the cloth simulation without any rendering
/// using Position based dynamics
///
int main()
{
    auto sdk = std::make_shared<SimulationManager>(false);
    auto scene = sdk->createNewScene("NoRendering");

    // Create surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    StdVectorOfVec3d vertList;
    const double width = 10.0;
    const double height = 10.0;
    static const int nRows = 5;
    static const int nCols = 5;
    const double epsilon = 1e-5;
    vertList.resize(nRows*nCols);
    const double dy = width / (double)(nCols - 1);
    const double dx = height / (double)(nRows - 1);
    std::cout << " initialPositions = {" << std::endl;
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertList[i*nCols + j] = Vec3d((double)dx*i, 1.0, (double)dy*j);
            std::cout << vertList[i*nCols + j][0] << ", "
                      << vertList[i*nCols + j][1] << ", "
                      << vertList[i*nCols + j][2] << ",    ";
        }
        std::cout << std::endl;
    }
    std::cout << "}" << std::endl;
    surfMesh->setInitialVertexPositions(vertList);
    surfMesh->setVertexPositions(vertList);

    // Add connectivity data
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

    // Create Object & Model
    auto deformableObj = std::make_shared<PbdObject>("Cloth");
    auto pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(surfMesh);

    // configure model
    auto pbdParams = std::make_shared<PBDModelConfig>();

    // Constraints
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 0.1);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 0.001);
    pbdParams->m_fixedNodeIds = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    // Other parameters
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity = Vec3d(0, -9.8, 0);
    pbdParams->m_dt = 0.03;
    pbdParams->m_maxIter = 5;

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

    // Add in scene
    scene->addSceneObject(deformableObj);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    // Method to call after the simulation is done running
    static StdVectorOfVec3d lastPositions; // Vertex positions at the last iteration
    lastPositions.resize(nRows*nCols);
    static StdVectorOfVec3d beforeLastPositions; // Vertex positions at the (N-1) iteration
    beforeLastPositions.resize(nRows*nCols);

    auto func =
        [&surfMesh](Module* module)
        {
            auto newPositions = surfMesh->getVertexPositions();
            for (int i = 0; i < nRows; ++i)
            {
                for (int j = 0; j < nCols; j++)
                {
                    beforeLastPositions[i*nCols + j] = lastPositions[i*nCols + j];
                    lastPositions[i*nCols + j] = newPositions[i*nCols + j];
                }
            }
        };
    sdk->getSceneManager(scene)->setPostUpdateCallback(func);

    // Start
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::RUNNING);

    // Sleep
    std::this_thread::sleep_for(std::chrono::seconds(300));

    // End
    sdk->endSimulation();

    const std::vector<Vec3d> expectedFinalPositions = {
        Vec3d(0, 1, 0),
        Vec3d(0, 1, 2.5),
        Vec3d(0, 1, 5),
        Vec3d(0, 1, 7.5),
        Vec3d(0, 1, 10),
        Vec3d(2.5, 1, 0),
        Vec3d(2.5, 1, 2.5),
        Vec3d(2.5, 1, 5),
        Vec3d(2.5, 1, 7.5),
        Vec3d(2.5, 1, 10),
        Vec3d(5, 1, 0),
        Vec3d(4.82032, 0.151579, 2.47237),
        Vec3d(4.44397, -0.540266, 4.95431),
        Vec3d(3.90422, -1.05503, 7.38393),
        Vec3d(2.84115, -1.52265, 9.57745),
        Vec3d(5.42292, -1.55528, -0.406832),
        Vec3d(5.15428, -2.3468, 2.05675),
        Vec3d(4.66615, -3.00741, 4.48803),
        Vec3d(3.91197, -3.54776, 6.83235),
        Vec3d(2.99625, -4.06199, 9.09933),
        Vec3d(5.48704, -4.10154, -0.904475),
        Vec3d(5.14791, -4.83293, 1.51491),
        Vec3d(4.62851, -5.46666, 3.92927),
        Vec3d(3.94636, -6.01879, 6.29803),
        Vec3d(3.13296, -6.54906, 8.61026),
    };

    bool sameLastStateSuccess = true;
    bool expectedLastStateSuccess = true;
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            std::cout << lastPositions[i*nCols + j][0] << ", " << lastPositions[i*nCols + j][1] << ", " << lastPositions[i*nCols + j][2] << ", " << std::endl;
        }
    }

    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            for (int k = 0; k < 3; ++k)
            {
                sameLastStateSuccess &= vtkMathUtilities::FuzzyCompare(
                  beforeLastPositions[i*nCols + j][k], lastPositions[i*nCols + j][k], epsilon);

                expectedLastStateSuccess &= vtkMathUtilities::FuzzyCompare(
                  lastPositions[i*nCols + j][k], expectedFinalPositions[i*nCols + j][k]);
                if (!expectedLastStateSuccess)
                {
                    std::cout << lastPositions[i*nCols + j][k] << "  " << expectedFinalPositions[i*nCols + j][k]<< std::endl;
                    expectedLastStateSuccess = true;
                }
            }
        }
    }

    if (!sameLastStateSuccess)
    {
        std::cerr << "Error: simulation did not converge" << std::endl;
        return EXIT_FAILURE;
    }
    if (!expectedLastStateSuccess)
    {
        std::cerr << "Error: last state positions are wrong" << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
