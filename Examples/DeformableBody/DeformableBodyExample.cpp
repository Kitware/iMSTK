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
#include "imstkConjugateGradient.h"
#include "imstkLight.h"

using namespace imstk;

///
/// \brief This example demonstrates the soft body simulation
/// using Finite elements
///
int
main()
{
    // simManager and Scene
    auto simConfig = std::make_shared<simManagerConfig>();
    simConfig->simulationMode = SimulationMode::Rendering;
    auto simManager = std::make_shared<SimulationManager>(simConfig);
    auto scene      = simManager->createNewScene("DeformableBodyFEM");
    scene->getCamera()->setPosition(0, 2.0, 15.0);

    // Load a tetrahedral mesh
    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");

    CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

    // Extract the surface mesh
    auto surfMesh   = std::make_shared<SurfaceMesh>();
    auto volTetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);

    CHECK(volTetMesh != nullptr) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";

    volTetMesh->extractSurfaceMesh(surfMesh, true);

    // Construct a map

    // Construct one to one nodal map based on the above meshes
    auto oneToOneNodalMap = std::make_shared<OneToOneMap>(tetMesh, surfMesh);

    // Scene object 1: Dragon

    // Configure dynamic model
    auto dynaModel = std::make_shared<FEMDeformableBodyModel>();

    //dynaModel->configure(iMSTK_DATA_ROOT "/asianDragon/asianDragon.config");

    auto config = std::make_shared<FEMModelConfig>();
    config->m_fixedNodeIds = { 51, 127, 178 };
    dynaModel->configure(config);

    dynaModel->setTimeStepSizeType(TimeSteppingType::Fixed);
    dynaModel->setModelGeometry(volTetMesh);
    auto timeIntegrator = std::make_shared<BackwardEuler>(0.05);// Create and add Backward Euler time integrator
    dynaModel->setTimeIntegrator(timeIntegrator);

    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    // Scene Object
    auto deformableObj = std::make_shared<FeDeformableObject>("Dragon");
    deformableObj->addVisualModel(surfMeshModel);
    deformableObj->setPhysicsGeometry(volTetMesh);
    deformableObj->setPhysicsToVisualMap(oneToOneNodalMap); //assign the computed map
    deformableObj->setDynamicalModel(dynaModel);
    scene->addSceneObject(deformableObj);

    // Scene object 2: Plane
    auto planeGeom = std::make_shared<Plane>();
    planeGeom->setWidth(40);
    planeGeom->setPosition(0, -6, 0);
    auto planeObj = std::make_shared<CollidingObject>("Plane");
    planeObj->setVisualGeometry(planeGeom);
    planeObj->setCollidingGeometry(planeGeom);
    scene->addSceneObject(planeObj);

    // create a nonlinear system
    auto nlSystem = std::make_shared<NonLinearSystem>(
        dynaModel->getFunction(),
        dynaModel->getFunctionGradient());

    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    // create a linear solver
    auto linSolver = std::make_shared<ConjugateGradient>();

    if (linSolver->getType() == imstk::LinearSolver<imstk::SparseMatrixd>::Type::GaussSeidel
        && dynaModel->isFixedBCImplemented())
    {
        LOG(WARNING) << "The GS solver may not be viable!";
    }

    // create a non-linear solver and add to the scene
    auto nlSolver = std::make_shared<NewtonSolver>();
    nlSolver->setLinearSolver(linSolver);
    nlSolver->setSystem(nlSystem);
    scene->addNonlinearSolver(nlSolver);

    // Light
    auto light = std::make_shared<DirectionalLight>("light");
    light->setFocalPoint(Vec3d(5, -8, -5));
    light->setIntensity(1);
    scene->addLight(light);

    // Run the simulation
    simManager->setActiveScene(scene);
    simManager->start(SimulationStatus::Paused);

    return 0;
}
