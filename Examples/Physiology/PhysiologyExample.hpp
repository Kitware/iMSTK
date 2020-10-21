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

#include "Fluid.hpp"

#include "imstkNew.h"
#include "imstkCamera.h"
#include "imstkCollisionDetection.h"
#include "imstkCollisionGraph.h"
#include "imstkHemorrhageAction.h"
#include "imstkLight.h"
#include "imstkPhysiologyModel.h"
#include "imstkPhysiologyObject.h"
//#include "imstkPlane.h"
#include "imstkPointSet.h"
#include "imstkObjectInteractionFactory.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSphere.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"
#include "imstkSPHPhysiologyInteraction.h"
#include "imstkTaskGraph.h"
//#include "imstkVTKTextStatusManager.h"
#include "imstkVTKViewer.h"
#include "imstkMouseSceneControl.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"

using namespace imstk;

static std::shared_ptr<PhysiologyObject> 
makePhysiologyObject()
{
    // configure model
    auto physiologyParams = std::make_shared<PhysiologyModelConfig>();

    // Create a physics model
    auto physiologyModel = std::make_shared<PhysiologyModel>();
    physiologyModel->configure(physiologyParams);

    // Setup hemorrhage action
    auto hemorrhageAction = std::make_shared<HemorrhageAction>(HemorrhageAction::Type::External, "VascularCompartment::RightLeg");
    physiologyModel->addAction("Hemorrhage", hemorrhageAction);

    auto physiologyObj = std::make_shared<PhysiologyObject>("Pulse");
    physiologyObj->setDynamicalModel(physiologyModel);

    return physiologyObj;
}

static void 
parseArguments(int argc, char* argv[], double& particleRadius, int& numThreads)
{
    if (SCENE_ID == 5)
    {
        particleRadius = 0.012;
    }
    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        auto param = std::string(argv[i]);
        if (param.find("threads") == 0
            && param.find_first_of("=") != std::string::npos)
        {
            numThreads = std::stoi(param.substr(param.find_first_of("=") + 1));
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
}

int
main(int argc, char* argv[])
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("SPHPhysiologyInteraction");


    int threads = -1;
    double particleRadius = 0.04;
    parseArguments(argc, argv, particleRadius, threads);

    // Generate fluid and solid objects
    auto fluidObj = generateFluid(scene, particleRadius);
    auto material = fluidObj->getVisualModel(0)->getRenderMaterial();
    auto sphModel = fluidObj->getDynamicalSPHModel();


    std::shared_ptr<PhysiologyObject> physioObj = makePhysiologyObject();
    scene->addSceneObject(physioObj);

    auto interactionPair = std::make_shared<SPHPhysiologyObjectInteractionPair>(fluidObj, physioObj);

    // configure the sph-physiology interaction pair
    interactionPair->setHemorrhageAction(
        std::dynamic_pointer_cast<HemorrhageAction>(physioObj->getPhysiologyModel()->getAction("Hemorrhage")));
    interactionPair->setCompartment(PhysiologyCompartmentType::Liquid, "VascularCompartment::RightLeg");

    scene->getCollisionGraph()->addInteraction(interactionPair);

    // configure camera
    (SCENE_ID == 5) ? scene->getActiveCamera()->setPosition(0, 1.0, 4.0) : scene->getActiveCamera()->setPosition(0, 1.0, 5.0);

    // configure light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);
    
    // Setup some scalars
    std::shared_ptr<PointSet> fluidGeometry = std::dynamic_pointer_cast<PointSet>(fluidObj->getPhysicsGeometry());
    std::shared_ptr<StdVectorOfReal> scalarsPtr = std::make_shared<StdVectorOfReal>(fluidGeometry->getNumVertices());
    std::fill_n(scalarsPtr->data(), scalarsPtr->size(), 0.0);
    fluidGeometry->setScalars(scalarsPtr);

    // Setup the material for the scalars
    material->setScalarVisibility(true);
    std::shared_ptr<ColorFunction> colorFunc = std::make_shared<ColorFunction>();
    colorFunc->setNumberOfColors(2);
    colorFunc->setColor(0, Color::Red);
    colorFunc->setColor(1, Color::Green);
    colorFunc->setColorSpace(ColorFunction::ColorSpace::RGB);
    colorFunc->setRange(0, 3);
    material->setColorLookupTable(colorFunc);

    auto displayColors = [&](Event*)
    {
        auto taskGraph = scene->getTaskGraph();
        //taskGraph->removeNode(fluidObj->getDynamicalSPHModel()->getComputeSurfaceTensionNode());

        std::shared_ptr<TaskNode> printTotalTime = std::make_shared<TaskNode>([&]()
            {
                if (fluidObj->getDynamicalSPHModel()->getTimeStepCount() % 100 == 0)
                {
                    printf("Total time (s): %f\n", fluidObj->getDynamicalSPHModel()->getTotalTime());
                }
            }, "PrintTotalTime");
        taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getMoveParticlesNode(), printTotalTime);

        std::shared_ptr<TaskNode> writeSPHStateToCSV = std::make_shared<TaskNode>([&]() {
            fluidObj->getDynamicalSPHModel()->writeStateToCSV();
            }, "WriteStateToCSV");
        taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getMoveParticlesNode(), writeSPHStateToCSV);

        std::shared_ptr<TaskNode> writeSPHStateToVtk = std::make_shared<TaskNode>([&]() {
            fluidObj->getDynamicalSPHModel()->writeStateToVtk();
            }, "WriteStateToVtk");
        taskGraph->insertAfter(fluidObj->getDynamicalSPHModel()->getMoveParticlesNode(), writeSPHStateToVtk);

        // This node colors the fluid points based on their type
        std::shared_ptr<TaskNode> computeVelocityScalars = std::make_shared<TaskNode>([&]() {
            const std::shared_ptr<SPHBoundaryConditions> sphBoundaryConditions = sphModel->getBoundaryConditions();
            StdVectorOfReal& scalars = *scalarsPtr;
            for (size_t i = 0; i < sphModel->getCurrentState()->getNumParticles(); i++)
            {
                if (sphBoundaryConditions->getParticleTypes()[i] == SPHBoundaryConditions::ParticleType::Wall)
                {
                    scalars[i] = 0;
                }
                else if (sphBoundaryConditions->getParticleTypes()[i] == SPHBoundaryConditions::ParticleType::Inlet)
                {
                    scalars[i] = 1;
                }
                else if (sphBoundaryConditions->getParticleTypes()[i] == SPHBoundaryConditions::ParticleType::Outlet)
                {
                    scalars[i] = 2;
                }
                else
                {
                    scalars[i] = 3;
                }
            }
            }, "ComputeVelocityScalars");
        taskGraph->insertAfter(fluidObj->getUpdateGeometryNode(), computeVelocityScalars);
    };

    // Setup a viewer to render in its own thread
    imstkNew<VTKViewer> viewer("Viewer");
    viewer->setActiveScene(scene);

    // Setup a scene manager to advance the scene in its own thread
    imstkNew<SceneManager> sceneManager("Scene Manager");
    sceneManager->setActiveScene(scene);
    viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer
    connect<Event>(sceneManager, EventType::PostUpdate, displayColors);

    // Add mouse and keyboard controls to the viewer
    {
        imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        viewer->addControl(mouseControl);

        imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setViewer(viewer);
    }

    // Start viewer running, scene as paused
    sceneManager->requestStatus(ThreadStatus::Running);
    viewer->start();

    return 0;
}
