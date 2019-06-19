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

#include "imstkSimulationManager.h"
#include "imstkSPHObject.h"
#include "imstkAPIUtilities.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

using namespace imstk;

std::shared_ptr<SPHObject> generateFluid(const std::shared_ptr<Scene>&scene, int sceneIdx, const double particleRadius);
std::vector<std::shared_ptr<CollidingObject>> generateSolids(const std::shared_ptr<Scene>& scene, int sceneIdx);

///
/// \brief Usage: ./SPHFluid [threads=<num_threads>] [radius=<particle_radius>]
/// \brief Example: ./SPHFluid threads=8 radius=0.01
///
int main(int argc, char* argv[])
{
    // SimulationManager must be created first
    auto sdk = std::make_shared<SimulationManager>(0);

    int threads = -1;
    double particleRadius = 0.1;

    // Parse command line arguments
    for(int i = 1; i < argc; ++i)
    {
        auto param = std::string(argv[i]);
        if(param.find("threads") == 0 &&
           param.find_first_of("=") != std::string::npos)
        {
            threads = std::stoi(param.substr(param.find_first_of("=") + 1));
        }
        else if(param.find("radius") == 0 &&
                param.find_first_of("=") != std::string::npos)
        {
            particleRadius = std::stod(param.substr(param.find_first_of("=") + 1));
            LOG(INFO) << "Particle radius: " << particleRadius;
        }
        else
        {
            LOG(FATAL) << "Invalid argument";
        }
    }

    // Override particle radius for scene3 because particles in this scene were pre-generated using particle radius 0.08
    if(SCENE_ID == 3)
    {
        particleRadius = 0.08;
    }

    // Set thread pool size (nthreads <= 0 means using all logical cores)
    sdk->setThreadPoolSize(threads);

    auto scene = sdk->createNewScene("SPH Fluid");

    // Generate fluid and solid objects
    auto fluidObj = generateFluid(scene, SCENE_ID, particleRadius);
    auto solids = generateSolids(scene, SCENE_ID);

    // Collision between fluid and solid objects
    auto colGraph = scene->getCollisionGraph();

    for(auto& solid: solids)
    {
        if(std::dynamic_pointer_cast<Plane>(solid->getCollidingGeometry()))
        {
            colGraph->addInteractionPair(fluidObj, solid,
                                 CollisionDetection::Type::PointSetToPlane,
                                 CollisionHandling::Type::SPH,
                                 CollisionHandling::Type::None);
        }
        else if(std::dynamic_pointer_cast<Sphere>(solid->getCollidingGeometry()))
        {
            colGraph->addInteractionPair(fluidObj, solid,
                                         CollisionDetection::Type::PointSetToSphere,
                                         CollisionHandling::Type::SPH,
                                         CollisionHandling::Type::None);
        }
        else
        {
            LOG(FATAL) << "Invalid collision object";
        }
    }

    // configure camera
    scene->getCamera()->setPosition(0, 10.0, 15.0);

    // configure light (white)
    auto whiteLight = std::make_shared<DirectionalLight>("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5, -8, -5));
    whiteLight->setIntensity(7);
    scene->addLight(whiteLight);

    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
