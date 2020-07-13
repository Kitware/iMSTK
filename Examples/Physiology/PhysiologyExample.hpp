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

#include <string>

#include "imstkAPIUtilities.h"
#include "imstkCamera.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionGraph.h"
#include "imstkLight.h"
#include "imstkPlane.h"
#include "imstkPointSet.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSphere.h"
#include "imstkTaskGraph.h"
#include "imstkVTKTextStatusManager.h"
#include "imstkObjectInteractionFactory.h"

#include "imstkPhysiologyModel.h"
#include "imstkPhysiologyObject.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"

#include "Fluid.hpp"
#include "Solid.hpp"


using namespace imstk;

int
main(int argc, char* argv[])
{
  // SimulationManager must be created first
  auto simManager = std::make_shared<SimulationManager>();

  int    threads = -1;
  double particleRadius = 0.1;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i)
  {
    auto param = std::string(argv[i]);
    if (param.find("threads") == 0
      && param.find_first_of("=") != std::string::npos)
    {
      threads = std::stoi(param.substr(param.find_first_of("=") + 1));
    }
    else if (param.find("radius") == 0
      && param.find_first_of("=") != std::string::npos)
    {
      particleRadius = std::stod(param.substr(param.find_first_of("=") + 1));
      LOG(INFO) << "Particle radius: " << particleRadius;
    }
    else
    {
      LOG(FATAL) << "Invalid argument";
    }
  }

  // Set thread pool size (nthreads <= 0 means using all logical cores)
  simManager->setThreadPoolSize(threads);

  auto scene = simManager->createNewScene("SPHPhysiologyInteraction");

  scene->getConfig()->writeTaskGraph = true;
  scene->getConfig()->taskTimingEnabled = true;
  //scene->getTaskComputeTimes().at("PhysiologyModel_Solve");

  // Get the VTKViewer
  auto viewer = std::dynamic_pointer_cast<VTKViewer>(simManager->getViewer());
  viewer->getVtkRenderWindow()->SetSize(1920, 1080);

  auto statusManager = viewer->getTextStatusManager();
  statusManager->setStatusFontSize(VTKTextStatusManager::Custom, 30);
  statusManager->setStatusFontColor(VTKTextStatusManager::Custom, Color::Red);

  // Generate fluid and solid objects
  auto fluidObj = generateFluid(scene, particleRadius);
  //auto solids = generateSolids(scene);

  //simManager->getSceneManager(scene)->setPostUpdateCallback([&](Module*) {
  //  statusManager->setCustomStatus("Number of particles: " +
  //    std::to_string(fluidObj->getSPHModel()->getState().getNumParticles()) +
  //    "\nNumber of solids: " + std::to_string(solids.size()));
  //  });

    // configure model
  auto physiologyParams = std::make_shared<PhysiologyModelConfig>();

  // Create a physics model
  auto physiologyModel = std::make_shared<PhysiologyModel>();
  physiologyModel->configure(physiologyParams);
  auto physiologyObj = std::make_shared<PhysiologyObject>("Pulse");
  physiologyObj->setDynamicalModel(physiologyModel);

  scene->addSceneObject(physiologyObj);

  scene->getCollisionGraph()->addInteraction(makeObjectInteractionPair(fluidObj, physiologyObj,
    InteractionType::SphObjToPhysObjCollision, CollisionDetection::Type::Custom));

  // configure camera
  scene->getCamera()->setPosition(0, 5.0, 20.0);

  // configure light (white)
  auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
  whiteLight->setFocalPoint(Vec3d(5, -8, -5));
  whiteLight->setIntensity(7);
  scene->addLight(whiteLight);

  simManager->setActiveScene(scene);

  // remove the surface tension node
  scene->setTaskGraphConfigureCallback([&](Scene* scene)
    {
      auto taskGraph = scene->getTaskGraph();
      taskGraph->removeNode(fluidObj->getDynamicalSPHModel()->getComputeSurfaceTensionNode());

      std::shared_ptr<TaskNode> printTotalTime = std::make_shared<TaskNode>([&]()
        {
          if (fluidObj->getDynamicalSPHModel()->getTimeStepCount() % 100 == 0)
          {
            printf("Total time (s): %f\n", fluidObj->getDynamicalSPHModel()->getTotalTime());
          }
        }, "PrintTotalTime");

      taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getIntegrateNode(), printTotalTime);

      std::shared_ptr<TaskNode> writeSPHStateToCSV = std::make_shared<TaskNode>([&]() {
        fluidObj->getDynamicalSPHModel()->writeStateToCSV();
        }, "WriteStateToCSV");
      taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getIntegrateNode(), writeSPHStateToCSV);

      std::shared_ptr<TaskNode> writeSPHStateToVtk = std::make_shared<TaskNode>([&]() {
        fluidObj->getDynamicalSPHModel()->writeStateToVtk();
        }, "WriteStateToVtk");
      taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getIntegrateNode(), writeSPHStateToVtk);

      std::shared_ptr<TaskNode> printSPHParticleTypes = std::make_shared<TaskNode>([&]() {
        if (fluidObj->getDynamicalSPHModel()->getTimeStepCount() % 100 == 0)
        {
          fluidObj->getDynamicalSPHModel()->printParticleTypes();
        }
        }, "PrintSPHParticleTypes");
      taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getIntegrateNode(), printSPHParticleTypes);
    });

  simManager->start(SimulationStatus::Paused);

  return 0;
}
