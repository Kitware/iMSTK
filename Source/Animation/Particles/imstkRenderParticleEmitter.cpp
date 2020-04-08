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

#include "imstkRenderParticleEmitter.h"
#include "imstkColor.h"
#include "imstkLogUtility.h"

namespace imstk
{
RenderParticleEmitter::RenderParticleEmitter(std::shared_ptr<Geometry>   geometry,
                                             const float                 time /*= 3000*/,
                                             RenderParticleEmitter::Mode mode /*= Mode::CONTINUOUS*/)
    : AnimationModel(geometry)
{
    this->setGeometry(geometry);

    m_time     = time;
    m_emitTime = m_time;
    m_mode     = mode;

    RenderParticleKeyFrame startFrame;
    startFrame.m_color = Color::White;
    startFrame.m_time  = 0.0f;
    startFrame.m_acceleration = Vec3f(0, 0, 0);
    startFrame.m_rotationalAcceleration = 0.0f;

    RenderParticleKeyFrame endFrame;
    endFrame.m_color = startFrame.m_color;
    endFrame.m_time  = m_time;
    endFrame.m_acceleration = Vec3f(0, 0, 0);
    endFrame.m_rotationalAcceleration = 0.0f;

    m_keyFrames.push_back(startFrame);
    m_keyFrames.push_back(endFrame);

    this->initializeParticles();
}

void
RenderParticleEmitter::setGeometry(
    std::shared_ptr<Geometry> geometry)
{
    CHECK(geometry->getType() == Geometry::Type::RenderParticles) << "Geometry must be RenderParticles";

    m_animationGeometry = geometry;
    m_particles = &std::static_pointer_cast<RenderParticles>(m_animationGeometry)->getParticles();
}

RenderParticleEmitter::Mode
RenderParticleEmitter::getEmitterMode() const
{
    return m_mode;
}

void
RenderParticleEmitter::setEmitterSize(const float size)
{
    m_emitterSize = size;
}

void
RenderParticleEmitter::setInitialVelocityRange(const Vec3f minDirection,
                                               const Vec3f maxDirection,
                                               const float minSpeed,
                                               const float maxSpeed,
                                               const float minRotationSpeed,
                                               const float maxRotationSpeed)
{
    m_minDirection = minDirection;
    m_maxDirection = maxDirection;
    m_minDirection.normalize();
    m_maxDirection.normalize();
    m_minSpeed = minSpeed;
    m_maxSpeed = maxSpeed;
    m_minRotationSpeed = minRotationSpeed;
    m_maxRotationSpeed = maxRotationSpeed;
}

bool
RenderParticleEmitter::addKeyFrame(RenderParticleKeyFrame keyFrame)
{
    if (m_keyFrames.size() >= c_maxNumKeyFrames)
    {
        return false;
    }

    m_keyFrames.push_back(keyFrame);
    return true;
}

RenderParticleKeyFrame*
RenderParticleEmitter::getStartKeyFrame()
{
    unsigned int index = 0;

    for (unsigned int i = 0; i < m_keyFrames.size(); i++)
    {
        if (m_keyFrames[i].m_time < m_keyFrames[index].m_time)
        {
            index = i;
        }
    }

    return &m_keyFrames[index];
}

RenderParticleKeyFrame*
RenderParticleEmitter::getEndKeyFrame()
{
    unsigned int index = 0;

    for (unsigned int i = 0; i < m_keyFrames.size(); i++)
    {
        if (m_keyFrames[i].m_time > m_keyFrames[index].m_time)
        {
            index = i;
        }
    }

    return &m_keyFrames[index];
}

std::vector<RenderParticleKeyFrame>&
RenderParticleEmitter::getKeyFrames()
{
    return m_keyFrames;
}

void
RenderParticleEmitter::reset()
{
    if (m_mode != Mode::Burst)
    {
        return;
    }

    auto renderParticles = std::static_pointer_cast<RenderParticles>(m_geometry);
    renderParticles->reset();

    this->initializeParticles();
}

void
RenderParticleEmitter::update()
{
    auto renderParticles = std::static_pointer_cast<RenderParticles>(m_geometry);

    if (!m_started)
    {
        m_stopWatch.start();
        m_started = true;
    }

    auto  time = m_stopWatch.getTimeElapsed();
    float dt   = (float)(time - m_lastUpdateTime);
    m_lastUpdateTime = time;

    for (auto&& particle : (*m_particles))
    {
        auto startKeyFrameTemp = *this->getStartKeyFrame();
        auto startKeyFrame     = &startKeyFrameTemp;
        auto endKeyFrameTemp   = *this->getEndKeyFrame();
        auto endKeyFrame       = &endKeyFrameTemp;

        particle->m_age += dt;

        if (!(particle->m_created) && particle->m_age >= 0)
        {
            particle->m_created = true;
            this->emitParticle(particle);
            renderParticles->incrementNumOfParticles();
        }
        else if (particle->m_age < 0)
        {
            continue;
        }

        if (m_mode == RenderParticleEmitter::Mode::Continuous
            && particle->m_age > m_time)
        {
            particle->m_age = particle->m_age - ((int)(particle->m_age / m_time) * m_time);
            this->emitParticle(particle);
        }

        // Search for nearest keyframe
        for (unsigned int i = 0; i < m_keyFrames.size(); i++)
        {
            auto keyFrame = &m_keyFrames[i];
            if (particle->m_age >= keyFrame->m_time
                && keyFrame->m_time > startKeyFrame->m_time)
            {
                startKeyFrame = keyFrame;
            }
            if (particle->m_age < keyFrame->m_time
                && keyFrame->m_time < endKeyFrame->m_time)
            {
                endKeyFrame = keyFrame;
            }
        }

        // Update velocity and position
        particle->m_rotationalAcceleration = startKeyFrame->m_rotationalAcceleration;
        particle->m_rotationalVelocity    += particle->m_rotationalAcceleration * (dt / 1000.0f);
        particle->m_rotation += particle->m_rotationalVelocity * (dt / 1000.0f);

        particle->m_acceleration = startKeyFrame->m_acceleration;
        particle->m_velocity    += particle->m_acceleration * (dt / 1000.0);
        particle->m_position    += particle->m_velocity * (dt / 1000.0);

        float timeDifference = endKeyFrame->m_time - startKeyFrame->m_time;
        float alpha = (particle->m_age - startKeyFrame->m_time) / timeDifference;

        particle->m_scale = (alpha * endKeyFrame->m_scale)
                            + ((1.0f - alpha) * startKeyFrame->m_scale);

        this->interpolateColor(particle->m_color,
            endKeyFrame->m_color,
            startKeyFrame->m_color,
            alpha);
    }
}

void
RenderParticleEmitter::initializeParticles()
{
    m_particles->clear();

    auto particles = std::static_pointer_cast<RenderParticles>(m_animationGeometry);

    for (unsigned int i = 0; i < particles->getMaxNumParticles(); i++)
    {
        m_particles->push_back(std::unique_ptr<RenderParticle>(new RenderParticle()));
        (*m_particles)[i]->m_age     = -(i / (float)(particles->getMaxNumParticles())) * m_emitTime;
        (*m_particles)[i]->m_created = false;
    }
}

void
RenderParticleEmitter::emitParticle(std::unique_ptr<RenderParticle>& particle)
{
    auto position = m_animationGeometry->getTranslation();

    if (m_shape == Shape::Cube)
    {
        float x = (getRandomNormalizedFloat() - 0.5f) * m_emitterSize;
        float y = (getRandomNormalizedFloat() - 0.5f) * m_emitterSize;
        float z = (getRandomNormalizedFloat() - 0.5f) * m_emitterSize;

        particle->m_position[0] = (float)position[0] + x;
        particle->m_position[1] = (float)position[1] + y;
        particle->m_position[2] = (float)position[2] + z;
    }

    float randomRotation = getRandomNormalizedFloat() * (float)imstk::PI * 2.0f;
    float randomRotationalVelocity = getRandomNormalizedFloat();
    particle->m_rotation = randomRotation;
    particle->m_rotationalVelocity = (randomRotationalVelocity * m_minRotationSpeed) +
                                     ((1.0f - randomRotationalVelocity) * m_maxRotationSpeed);

    float randomDirectionX = getRandomNormalizedFloat();
    float randomDirectionY = getRandomNormalizedFloat();
    float randomDirectionZ = getRandomNormalizedFloat();
    float randomSpeed      = getRandomNormalizedFloat();

    float speed      = (randomSpeed * m_minSpeed) + ((1.0f - randomSpeed) * m_maxSpeed);
    float directionX = (randomDirectionX * m_minDirection[0]) + ((1.0f - randomDirectionX) * m_maxDirection[0]);
    float directionY = (randomDirectionY * m_minDirection[1]) + ((1.0f - randomDirectionY) * m_maxDirection[1]);
    float directionZ = (randomDirectionZ * m_minDirection[2]) + ((1.0f - randomDirectionZ) * m_maxDirection[2]);
    Vec3f direction(directionX, directionY, directionZ);
    direction.normalize();
    particle->m_velocity[0] = directionX * speed;
    particle->m_velocity[1] = directionY * speed;
    particle->m_velocity[2] = directionZ * speed;
}

void
RenderParticleEmitter::interpolateColor(Color&       destination,
                                        const Color& sourceA,
                                        const Color& sourceB,
                                        const float  alpha)
{
    destination.r = (sourceA.r * alpha) + (sourceB.r * (1.0f - alpha));
    destination.g = (sourceA.g * alpha) + (sourceB.g * (1.0f - alpha));
    destination.b = (sourceA.b * alpha) + (sourceB.b * (1.0f - alpha));
    destination.a = (sourceA.a * alpha) + (sourceB.a * (1.0f - alpha));
}

float
RenderParticleEmitter::getRandomNormalizedFloat()
{
    return (float)std::rand() / RAND_MAX;
}
}
