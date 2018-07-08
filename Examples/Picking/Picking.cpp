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
#include "imstkMeshIO.h"
#include "imstkAPIUtilities.h"
#include "imstkPickingCH.h"
#include "imstkTetrahedralMesh.h"
#include "imstkOneToOneMap.h"
#include "imstkBackwardEuler.h"
#include "imstkDeformableObject.h"
#include "imstkConjugateGradient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkHDAPIDeviceClient.h"
#include "imstkPointSetToSpherePickingCD.h"
#include "imstkSceneObjectController.h"

// global variables
const std::string phantomOmni1Name = "Phantom1";

using namespace imstk;

///
/// \brief This example demonstrates picking nodes of soft object.
/// NOTE: Requires GeoMagic Touch device
///
int main()
{
#ifndef iMSTK_USE_OPENHAPTICS
    std::cout << "LaparoscopicToolController example needs haptic device to be enabled at build time" << std::endl;
    return 1;
#else if
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("Picking");

    //----------------------------------------------------------
    // Create plane visual scene object
    //----------------------------------------------------------
    auto planeObj = apiutils::createVisualAnalyticalSceneObject(
        Geometry::Type::Plane, scene, "VisualPlane", 100, Vec3d(0., -20., 0.));

    //----------------------------------------------------------
    // Create Nidus FE deformable scene object
    //----------------------------------------------------------
    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/oneTet/oneTet.veg");

    if (!tetMesh)
    {
        LOG(WARNING) << "Could not read mesh from file.";
        return 1;
    }
    // Extract the surface mesh
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
    if (!volTetMesh)
    {
        LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
        return 1;
    }
    auto surfMesh = std::make_shared<SurfaceMesh>();
    volTetMesh->extractSurfaceMesh(surfMesh);

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>();
    oneToOneNodalMap->setMaster(tetMesh);
    oneToOneNodalMap->setSlave(surfMesh);
    oneToOneNodalMap->compute();

    // Configure the dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();
    dynaModel->configure(iMSTK_DATA_ROOT "/oneTet/oneTet.config");
    dynaModel->setModelGeometry(volTetMesh);

    // Create and add Backward Euler time integrator
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.01);
    dynaModel->setTimeIntegrator(timeIntegrator);

    // Configure Scene Object
    auto physicsObj = std::make_shared<DeformableObject>("deformableObj");
    physicsObj->setVisualGeometry(surfMesh);
    physicsObj->setCollidingGeometry(volTetMesh);
    physicsObj->setPhysicsGeometry(volTetMesh);
    physicsObj->setPhysicsToVisualMap(oneToOneNodalMap);
    physicsObj->setDynamicalModel(dynaModel);
    //physicsObj->initialize();
    scene->addSceneObject(physicsObj);

    //----------------------------------------------------------
    // Create a nonlinear system and its solver
    //----------------------------------------------------------
    auto nlSystem = std::make_shared<NonLinearSystem>(dynaModel->getFunction(), dynaModel->getFunctionGradient());
    std::vector<LinearProjectionConstraint> linProj;
    for (auto id : dynaModel->getFixNodeIds())
    {
        linProj.push_back(LinearProjectionConstraint(id, true));
    }
    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());
    std::vector<LinearProjectionConstraint> dynLinProj;

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    auto cgLinSolver = std::make_shared<ConjugateGradient>();// create a linear solver to be used in the NL solver
    cgLinSolver->setLinearProjectors(&linProj);
    cgLinSolver->setDynamicLinearProjectors(&dynLinProj);
    nlSolver->setLinearSolver(cgLinSolver);
    nlSolver->setSystem(nlSystem);
    scene->addNonlinearSolver(nlSolver);

    //----------------------------------------------------------
    // Create object controller
    //----------------------------------------------------------

    // Device clients
    auto client = std::make_shared<HDAPIDeviceClient>(phantomOmni1Name);

    // Device Server
    auto server = std::make_shared<HDAPIDeviceServer>();
    server->addDeviceClient(client);
    sdk->addModule(server);

    // Sphere0
    auto sphereForPickObj = apiutils::createCollidingAnalyticalSceneObject(
        Geometry::Type::Sphere, scene, "Sphere0", 1, Vec3d(0., 0., 0.));

    auto pickTrackingCtrl = std::make_shared<DeviceTracker>(client);
    //pickTrackingCtrl->setTranslationOffset(Vec3d(0., 0., 24.));

    auto pickController = std::make_shared<SceneObjectController>(sphereForPickObj, pickTrackingCtrl);
    scene->addObjectController(pickController);

    CollisionData coldata;
    auto sphereGeo = std::dynamic_pointer_cast<Sphere>(sphereForPickObj->getCollidingGeometry());

    // Create collision detection for picking
    auto pickingCD = std::make_shared<PointSetToSpherePickingCD>(volTetMesh, sphereGeo, coldata);
    pickingCD->setDeviceTrackerAndButton(pickTrackingCtrl, 0);

    // Create contact handling for picking
    auto pickingCH = std::make_shared<PickingCH>(CollisionHandling::Side::A, coldata, physicsObj);
    pickingCH->setDynamicLinearProjectors(&dynLinProj);

    // Create collision pair
    scene->getCollisionGraph()->addInteractionPair(physicsObj, sphereForPickObj, pickingCD, pickingCH, nullptr);

    // Set Camera configuration
    auto cam = scene->getCamera();
    auto camPosition = Vec3d(0, 40, 80);
    cam->setPosition(camPosition);
    cam->setFocalPoint(Vec3d(0, 0, 0));
    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
#endif
}
