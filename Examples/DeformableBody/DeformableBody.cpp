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

#include "imstkTimer.h"
#include "imstkSimulationManager.h"
#include "imstkForceModelConfig.h"
#include "imstkDeformableObject.h"
#include "imstkBackwardEuler.h"
#include "imstkNonlinearSystem.h"
#include "imstkNewtonSolver.h"
#include "imstkGaussSeidel.h"
#include "imstkPlane.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkOneToOneMap.h"
#include "imstkAPIUtilities.h"

using namespace imstk;

void testDeformableBody()
{
    // a. SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("DeformableBody");
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // b. Load a tetrahedral mesh
    //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/oneTet/oneTet.veg");
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT"/liver/liver.veg");
    //auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT"/oneTet/oneTet.veg");
    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return;
    }

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return;
    }
    volTetMesh->extractSurfaceMesh(surfMesh, true);

    StopWatch wct;
    CpuTimer cput;

    wct.start();
    cput.start();

    // d. Construct a map

    // d.1 Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);

    // d.2 Compute the map
    oneToOneNodalMap->compute();

    LOG(INFO) << "wall clock time: " << wct.getTimeElapsed() << " ms.";
    LOG(INFO) << "CPU time: " << cput.getTimeElapsed() << " ms.";

    // e. Scene object 1: Dragon

    // Configure dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    //dynaModel->configure(iMSTK_DATA_ROOT "/oneTet/oneTet.config");
    dynaModel->configure(iMSTK_DATA_ROOT "/asianDragon/asianDragon.config");
    dynaModel->setTimeStepSizeType(TimeSteppingType::realTime);
    //dynaModel->configure(iMSTK_DATA_ROOT"/liver/liver.config");
    dynaModel->setModelGeometry(volTetMesh);
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.001);// Create and add Backward Euler time integrator
    dynaModel->setTimeIntegrator(timeIntegrator);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME_SURFACE);
    surfMesh->setRenderMaterial(material);

    // Scene Object
    auto deformableObj = std::make_shared<DeformableObject>("Dragon");
    deformableObj->setVisualGeometry(surfMesh);
    //deformableObj->setCollidingGeometry(surfMesh);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map
    deformableObj->setDynamicalModel(dynaModel);
    scene->addSceneObject(deformableObj);

    // f. Scene object 2: Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(40);
    planeGeom->setPosition(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // g. Add collision detection
    //auto collisioDet = std::make_shared<CollisionDetection>();

    // h. Add collision handling

    // create a nonlinear system
    auto nlSystem = std::make_shared<NonLinearSystem>(
        dynaModel->getFunction(),
        dynaModel->getFunctionGradient());

    std::vector<LinearProjectionConstraint> projList;
    for (auto i : dynaModel->getFixNodeIds())
    {
        auto s = LinearProjectionConstraint(i, false);
        s.setProjectorToDirichlet(i, Vec3d(0.001, 0, 0));
        projList.push_back(s);
    }

    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a linear solver
    //auto linSolver = std::make_shared<ConjugateGradient>();
    auto linSolver = std::make_shared<GaussSeidel>();
    //auto linSolver = std::make_shared<Jacobi>();
    //auto linSolver = std::make_shared<SOR>(0.4);

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    linSolver->setLinearProjectors(&projList);
    nlSolver->setLinearSolver(linSolver);
    nlSolver->setSystem(nlSystem);
    scene->addNonlinearSolver(nlSolver);

    // print UPS
    auto ups = std::make_shared<UPSCounter>();
    apiutils::printUPS(sdk->getSceneManager(scene), ups);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);
}

int main()
{
    testDeformableBody();
    return 0;
}
