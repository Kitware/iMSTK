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

#include "imstkPointSet.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSPHObject.h"
#include "imstkSPHModel.h"
#include "imstkVisualModel.h"

using namespace imstk;

///
/// \brief Generate a sphere-shape fluid object
///
std::shared_ptr<VecDataArray<double, 3>>
generateSphereShapeFluid(const double particleRadius)
{
    const double sphereRadius = 2.0;
    const Vec3d  sphereCenter(0, 1, 0);

    const double  sphereRadiusSqr = sphereRadius * sphereRadius;
    const double  spacing = 2.0 * particleRadius;
    const int     N = static_cast<int>(2.0 * sphereRadius / spacing);              // Maximum number of particles in each dimension
    const Vec3d lcorner = sphereCenter - Vec3d(sphereRadius, sphereRadius, sphereRadius); // Cannot use auto here, due to Eigen bug

    std::shared_ptr<VecDataArray<double, 3>> particles = std::make_shared<VecDataArray<double, 3>>();
    particles->reserve(N * N * N);

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            for (int k = 0; k < N; ++k)
            {
                Vec3d ppos = lcorner + Vec3d(spacing * static_cast<double>(i), spacing * static_cast<double>(j), spacing * static_cast<double>(k));
                Vec3d cx = ppos - sphereCenter;
                if (cx.squaredNorm() < sphereRadiusSqr)
                {
                    particles->push_back(ppos);
                }
            }
        }
    }

    return particles;
}

///
/// \brief Generate a box-shape fluid object
///
std::shared_ptr<VecDataArray<double, 3>>
generateBoxShapeFluid(const double particleRadius)
{
    const double boxWidth = 4.0;
    const Vec3d  boxLowerCorner(-2, -3, -2);

    const double spacing = 2.0 * particleRadius;
    const int N = static_cast<int>(boxWidth / spacing);

    std::shared_ptr<VecDataArray<double, 3>> particles = std::make_shared<VecDataArray<double, 3>>();
    particles->reserve(N * N * N);

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            for (int k = 0; k < N; ++k)
            {
                Vec3d ppos = boxLowerCorner + Vec3d(spacing * static_cast<double>(i), spacing * static_cast<double>(j), spacing * static_cast<double>(k));
                particles->push_back(ppos);
            }
        }
    }

    return particles;
}

#if SCENE_ID == 3
std::shared_ptr<VecDataArray<double, 3>> getBunny(); // Defined in Bunny.cpp
#endif
///
/// \brief Generate a bunny-shape fluid object
///
std::shared_ptr<VecDataArray<double, 3>>
generateBunnyShapeFluid(const double particleRadius)
{
    LOG_IF(FATAL, (std::abs(particleRadius - 0.08) > 1e-6)) << "Particle radius for this scene must be 0.08";
    std::shared_ptr<VecDataArray<double, 3>> particles = std::make_shared<VecDataArray<double, 3>>();
#if SCENE_ID == 3
    particles = getBunny();
#endif
    return particles;
}

///
/// \brief Generate fluid for pipe flow
///
std::shared_ptr<VecDataArray<double, 3>>
generatePipeFluid(const double particleRadius)
{
    const double pipeRadius = 1.0;
    const double pipeLength = 5.0;
    const Vec3d  lcorner(-5.0, 5.0, 0.0);
    const Vec3d pipeLeftCenter = lcorner + Vec3d(0.0, pipeRadius, pipeRadius);

    const double spacing = 2.0 * particleRadius;
    const int N_width = static_cast<int>(2.0 * pipeRadius / spacing); // Maximum number of particles in width dimension
    const int N_length = static_cast<int>(pipeLength / spacing); // Maximum number of particles in length dimension

    imstkNew<VecDataArray<double, 3>> particlesPtr;
    VecDataArray<double, 3>& particles = *particlesPtr.get();
    particles.reserve(N_width * N_width * N_length);

    for (int i = 0; i < N_length; ++i)
    {
        for (int j = 0; j < N_width; ++j)
        {
            for (int k = 0; k < N_width; ++k)
            {
                Vec3d ppos = lcorner + Vec3d(spacing * static_cast<double>(i), spacing * static_cast<double>(j), spacing * static_cast<double>(k));
                //const double cx = ppos.x() - pipeBottomCenter.x();
                //const double cy = ppos.y() - pipeBottomCenter.y();
                Vec3d cx = ppos - Vec3d(spacing * static_cast<double>(i), 0.0, 0.0) - pipeLeftCenter;
                if (cx.squaredNorm() < pipeRadius)
                {
                    particles.push_back(ppos);
                }
            }
        }
    }

    return particlesPtr;
}

std::shared_ptr<VecDataArray<double, 3>>
initializeNonZeroVelocities(const int numParticles)
{
    imstkNew<VecDataArray<double, 3>> initVelocitiesPtr(numParticles);
    initVelocitiesPtr->fill(Vec3d(10.0, 0.0, 0.0));
    return initVelocitiesPtr;
}

std::shared_ptr<SPHObject>
generateFluid(const double particleRadius)
{
    std::shared_ptr<VecDataArray<double, 3>> particles = std::make_shared<VecDataArray<double, 3>>();
    switch (SCENE_ID)
    {
    case 1:
        particles = generateSphereShapeFluid(particleRadius);
        break;
    case 2:
        particles = generateBoxShapeFluid(particleRadius);
        break;
    case 3:
        particles = generateBunnyShapeFluid(particleRadius);
        break;
    default:
        LOG(FATAL) << "Invalid scene index";
    }

    LOG(INFO) << "Number of particles: " << particles->size();

    // Create a geometry object
    imstkNew<PointSet> geometry;
    geometry->initialize(particles);

    // Create a fluids object
    imstkNew<SPHObject> fluidObj("SPHSphere");

    // Create a visual model
    imstkNew<VisualModel> visualModel(geometry.get());
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    //material->setDisplayMode(RenderMaterial::DisplayMode::Points);
    if (material->getDisplayMode() == RenderMaterial::DisplayMode::Fluid)
    {
        material->setPointSize(0.1);
    }
    else
    {
        material->setPointSize(20.0);
        material->setRenderPointsAsSpheres(true);
        material->setColor(Color::Orange);
    }
    visualModel->setRenderMaterial(material);

    // Create a physics model
    imstkNew<SPHModel> sphModel;
    sphModel->setModelGeometry(geometry);

    // Configure model
    imstkNew<SPHModelConfig> sphParams(particleRadius);
    sphParams->m_bNormalizeDensity = true;
    if (SCENE_ID == 2)   // highly viscous fluid
    {
        sphParams->m_kernelOverParticleRadiusRatio = 6.0;
        sphParams->m_surfaceTensionStiffness = 5.0;
    }

    if (SCENE_ID == 3)   // bunny-shaped fluid
    {
        sphParams->m_frictionBoundary = 0.3;
    }

    sphModel->configure(sphParams);
    sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

    // Add the component models
    fluidObj->addVisualModel(visualModel);
    fluidObj->setCollidingGeometry(geometry);
    fluidObj->setDynamicalModel(sphModel);
    fluidObj->setPhysicsGeometry(geometry);

    return fluidObj;
}
