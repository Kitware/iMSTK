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

namespace imstk
{
RenderParticleEmitter::RenderParticleEmitter(unsigned int maxNumParticles /*=128*/,
                                             float time /*= 3000*/,
                                             RenderParticleEmitter::Mode mode /*= Mode::BURST*/)
    : Geometry(Geometry::Type::RenderParticleEmitter)
{
    if (maxNumParticles <= 128)
    {
        m_maxNumParticles = maxNumParticles;
    }
    else
    {
        m_maxNumParticles = 128;
        LOG(WARNING) << "The maximum number of decals is 128";
    }

    m_time = time;
    m_emitTime = m_time;
    m_mode = mode;

    RenderParticleKeyFrame startFrame;
    startFrame.m_color = Color::White;
    startFrame.m_time = 0.0f;
    startFrame.m_acceleration = Vec3f(0, 0, 0);
    startFrame.m_rotationalAcceleration = 0.0f;

    RenderParticleKeyFrame endFrame;
    endFrame.m_color = startFrame.m_color;
    endFrame.m_time = m_time;
    endFrame.m_acceleration = Vec3f(0, 0, 0);
    endFrame.m_rotationalAcceleration = 0.0f;

    m_keyFrames.push_back(startFrame);
    m_keyFrames.push_back(endFrame);

    m_vertexPositions[0] = glm::vec3(0.5, 0.5, 0);
    m_vertexPositions[1] = glm::vec3(0.5, -0.5, 0);
    m_vertexPositions[2] = glm::vec3(-0.5, 0.5, 0);
    m_vertexPositions[3] = glm::vec3(-0.5, -0.5, 0);

    m_vertexNormals[0] = glm::vec3(0.0, 0.0, 1.0);
    m_vertexNormals[1] = glm::vec3(0.0, 0.0, 1.0);
    m_vertexNormals[2] = glm::vec3(0.0, 0.0, 1.0);
    m_vertexNormals[3] = glm::vec3(0.0, 0.0, 1.0);

    m_vertexUVs[0] = glm::vec2(1.0, 1.0);
    m_vertexUVs[1] = glm::vec2(1.0, 0);
    m_vertexUVs[2] = glm::vec2(0, 1.0);
    m_vertexUVs[3] = glm::vec2(0, 0);

    m_triangles[0] = glm::ivec3(1, 0, 3);
    m_triangles[1] = glm::ivec3(0, 2, 3);

    this->initializeParticles();
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

RenderParticleEmitter::Mode
RenderParticleEmitter::getEmitterMode()
{
    return m_mode;
}

void
RenderParticleEmitter::setEmitterSize(float size)
{
    m_emitterSize = size;
}

void
RenderParticleEmitter::setParticleSize(float size)
{
    m_particleSize = size;
}

void
RenderParticleEmitter::updateParticleEmitter(Vec3d cameraPosition)
{
    if (!m_started)
    {
        m_stopWatch.start();
        m_started = true;
    }

    auto time = m_stopWatch.getTimeElapsed();
    float dt = (float)(time - m_lastUpdateTime);
    m_lastUpdateTime = time;

    for (auto&& particle : m_particles)
    {
        auto startKeyFrameTemp = *this->getStartKeyFrame();
        auto startKeyFrame = &startKeyFrameTemp;
        auto endKeyFrameTemp = *this->getEndKeyFrame();
        auto endKeyFrame = &endKeyFrameTemp;

        particle->m_age += dt;

        if (!(particle->m_created) && particle->m_age >= 0)
        {
            particle->m_created = true;
            this->emitParticle(particle);
            m_numParticles++;
        }
        else if (particle->m_age < 0)
        {
            continue;
        }

        if (m_mode == RenderParticleEmitter::Mode::CONTINUOUS
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
        particle->m_rotationalVelocity += particle->m_rotationalAcceleration * (dt / 1000.0f);
        particle->m_rotation += particle->m_rotationalVelocity * (dt / 1000.0f);

        particle->m_acceleration = startKeyFrame->m_acceleration;
        particle->m_velocity += particle->m_acceleration * (dt / 1000.0);
        particle->m_position += particle->m_velocity * (dt / 1000.0);

        float timeDifference = endKeyFrame->m_time - startKeyFrame->m_time;
        float alpha = (particle->m_age - startKeyFrame->m_time) / timeDifference;

        particle->m_scale = (alpha * endKeyFrame->m_scale)
                            + ((1.0f - alpha) * startKeyFrame->m_scale);

        interpolateColor(particle->m_color,
            endKeyFrame->m_color,
            startKeyFrame->m_color,
            alpha);
    }
}

void
RenderParticleEmitter::emitParticle(std::unique_ptr<RenderParticle>& particle)
{
    auto position = this->getTranslation();

    if (m_shape == Shape::CUBE)
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
    float randomSpeed = getRandomNormalizedFloat();

    float speed = (randomSpeed * m_minSpeed) + ((1.0f - randomSpeed) * m_maxSpeed);
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
RenderParticleEmitter::interpolateColor(Color& destination,
                                        Color& sourceA,
                                        Color& sourceB,
                                        float alpha)
{
    destination.r = (sourceA.r * alpha) + (sourceB.r * (1.0f - alpha));
    destination.g = (sourceA.g * alpha) + (sourceB.g * (1.0f - alpha));
    destination.b = (sourceA.b * alpha) + (sourceB.b * (1.0f - alpha));
    destination.a = (sourceA.a * alpha) + (sourceB.a * (1.0f - alpha));
}

unsigned int
RenderParticleEmitter::getNumParticles()
{
    return m_numParticles;
}

std::vector<std::unique_ptr<RenderParticle>>&
RenderParticleEmitter::getParticles()
{
    return m_particles;
}

std::vector<RenderParticleKeyFrame>&
RenderParticleEmitter::getKeyFrames()
{
    return m_keyFrames;
}

RenderParticleKeyFrame *
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

RenderParticleKeyFrame *
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

void
RenderParticleEmitter::setInitialVelocityRange(Vec3f minDirection,
                                               Vec3f maxDirection,
                                               float minSpeed,
                                               float maxSpeed,
                                               float minRotationSpeed,
                                               float maxRotationSpeed)
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

float
RenderParticleEmitter::getRandomNormalizedFloat()
{
    return (float)std::rand() / RAND_MAX;
}

void
RenderParticleEmitter::initializeParticles()
{
    m_particles.clear();

    for (unsigned int i = 0; i < m_maxNumParticles; i++)
    {
        m_particles.push_back(std::make_unique<RenderParticle>());
        m_particles[i]->m_age = -(i / (float)(m_maxNumParticles)) * m_emitTime;
        m_particles[i]->m_created = false;
    }
}

void
RenderParticleEmitter::reset()
{
    if (m_mode != Mode::BURST)
    {
        return;
    }

    m_numParticles = 0;

    this->initializeParticles();
}
}