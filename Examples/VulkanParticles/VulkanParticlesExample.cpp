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
#include "imstkRenderParticleEmitter.h"
#include "imstkAPIUtilities.h"

using namespace imstk;

///
/// \brief This example demonstrates particle rendering feature.
/// NOTE: Requires enabling Vulkan rendering backend
///
int main()
{
    // SDK and Scene
    auto sdk = std::make_shared<SimulationManager>();
    auto scene = sdk->createNewScene("RenderParticles");

    // Position camera
    auto cam = scene->getCamera();
    cam->setPosition(0, 3, 6);
    cam->setFocalPoint(0, 0, 0);

    // Smoke
    {
        auto particleMaterial = std::make_shared<RenderMaterial>();
        auto particleTexture = std::make_shared<Texture>
                               (iMSTK_DATA_ROOT "/particles/smoke_01.png", Texture::Type::DIFFUSE);
        particleMaterial->addTexture(particleTexture);
        particleMaterial->setBlendMode(RenderMaterial::BlendMode::ALPHA);

        auto particleEmitter = std::make_shared<RenderParticleEmitter>(128, 2000.0f);
        particleEmitter->setInitialVelocityRange(Vec3f(-1, 5, -1), Vec3f(1, 5, 1),
            0.5, 1.0,
            -1.0, 1.0);
        particleEmitter->setEmitterSize(0.3f);
        particleEmitter->setParticleSize(0.4f);

        auto startKeyFrame = particleEmitter->getStartKeyFrame();
        startKeyFrame->m_color = Color(1.0, 0.7, 0.0, 1.0);

        RenderParticleKeyFrame midFrame0;
        midFrame0.m_time = 700.0f;
        midFrame0.m_color = Color::Red;
        midFrame0.m_scale = 1.5f;
        particleEmitter->addKeyFrame(midFrame0);

        RenderParticleKeyFrame midFrame1;
        midFrame1.m_time = 1300.0f;
        midFrame1.m_color = Color::DarkGray;
        midFrame1.m_color.a = 0.7f;
        midFrame1.m_scale = 2.0f;
        particleEmitter->addKeyFrame(midFrame1);

        auto endKeyFrame = particleEmitter->getEndKeyFrame();
        endKeyFrame->m_color = Color::Black;
        endKeyFrame->m_color.a = 0.0;
        endKeyFrame->m_scale = 4.0;

        auto particleObject = std::make_shared<VisualObject>("Smoke");
        auto particleModel = std::make_shared<VisualModel>(particleEmitter);
        particleModel->setRenderMaterial(particleMaterial);
        particleObject->addVisualModel(particleModel);

        scene->addSceneObject(particleObject);
    }

    // Sparks
    {
        auto particleMaterial = std::make_shared<RenderMaterial>();
        auto particleTexture = std::make_shared<Texture>
                               (iMSTK_DATA_ROOT "/particles/flare_01.png", Texture::Type::DIFFUSE);
        particleMaterial->addTexture(particleTexture);
        particleMaterial->setBlendMode(RenderMaterial::BlendMode::ALPHA);

        auto particleEmitter = std::make_shared<RenderParticleEmitter>(128,
            850.0f, RenderParticleEmitter::Mode::BURST);
        particleEmitter->setTranslation(2, 0.1, 0);
        particleEmitter->setInitialVelocityRange(Vec3f(-1, 5, -1), Vec3f(1, 5, 1),
            4.0, 5.0,
            -1.0, 1.0);
        particleEmitter->setEmitterSize(0.1f);
        particleEmitter->setParticleSize(0.3f);

        auto startKeyFrame = particleEmitter->getStartKeyFrame();
        startKeyFrame->m_acceleration = Vec3f(0, -9.8, 0);
        startKeyFrame->m_color = Color::Yellow;

        auto endKeyFrame = particleEmitter->getEndKeyFrame();
        endKeyFrame->m_color = Color::Orange;

        auto particleObject = std::make_shared<VisualObject>("Sparks");
        auto particleModel = std::make_shared<VisualModel>(particleEmitter);
        particleModel->setRenderMaterial(particleMaterial);
        particleObject->addVisualModel(particleModel);

        scene->addSceneObject(particleObject);
    }

    // Plane
    auto plane = apiutils::createVisualAnalyticalSceneObject(Geometry::Type::Plane, scene, "plane", 10);
    plane->getVisualModel(0)->getRenderMaterial()->setColor(Color::Black);

    // Light
    auto light = std::make_shared<DirectionalLight>("Light");
    light->setIntensity(7);
    light->setColor(Color(1.0, 0.95, 0.8));
    light->setFocalPoint(Vec3d(-1, -1, 0));
    scene->addLight(light);

    auto viewer = sdk->getViewer();
    // Create a call back on key press of 'b' to trigger the sparks emitter
    viewer->setOnCharFunction('b', [&](InteractorStyle* c) -> bool
    {
        auto geometry = scene->getSceneObject("Sparks")->getVisualModel(0)->getGeometry();
        auto sparks = std::static_pointer_cast<RenderParticleEmitter>(geometry);
        sparks->reset();
        return false;
    });

    // Run
    sdk->setActiveScene(scene);
    sdk->startSimulation(SimulationStatus::PAUSED);

    return 0;
}
