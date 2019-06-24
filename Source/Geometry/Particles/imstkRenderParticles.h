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

#pragma once

#include <vector>
#include <climits>
#include <memory>

#include "glm/glm.hpp"

#include "imstkGeometry.h"
#include "imstkMath.h"
#include "imstkColor.h"
#include "imstkTimer.h"

namespace imstk
{
///
/// \struct RenderParticle
///
/// \brief Particle data
///
struct RenderParticle
{
    Vec3f m_position = Vec3f(0, 0, 0);
    Vec3f m_velocity = Vec3f(0, 0, 0);
    Vec3f m_acceleration = Vec3f(0, 0, 0);
    Color m_color = Color::White;
    float m_age = 0;
    bool m_created = false;
    float m_scale = 1.0f;
    float m_rotation = 0;
    float m_rotationalVelocity = 0;
    float m_rotationalAcceleration = 0;
};

///
/// \class RenderParticles
///
/// \brief Particles for rendering
///
class RenderParticles : public Geometry
{
public:

    ///
    /// \brief Constructor
    /// \param maxNumParticles Number of particles this emitter can produce
    /// \param time Lifespan of each particle (in milliseconds)
    /// \param mode Mode for emitter
    ///
    RenderParticles(const unsigned int maxNumParticles = 128);

    ///
    /// \brief Set size of particle
    /// \param size Particle size, this determines how much each keyframe
    ///        scales by
    ///
    void setParticleSize(const float size);

    ///
    /// \brief Get particles
    /// \returns particles
    ///
    std::vector<std::unique_ptr<RenderParticle>>& getParticles();

    ///
    /// \brief Reset number of particles
    ///
    void reset();

    ///
    /// \brief Increment number of particles
    ///
    void incrementNumOfParticles();

    ///
    /// \brief Get number of particles
    ///
    unsigned int getNumParticles();

    ///
    /// \brief Get maximum number of particles
    ///
    unsigned int getMaxNumParticles();

    ///
    /// \brief Get volume
    /// As these are particles, the volume is 0
    ///
    double getVolume() const override { return 0; };

protected:
    friend class VulkanParticleRenderDelegate;
    friend class RenderParticles;

    unsigned int m_maxNumParticles = 128; ///< Maximum particles
    float m_particleSize = 0.1f;

    std::vector<std::unique_ptr<RenderParticle>> m_particles; ///< Particle objects
    glm::vec3 m_vertexPositions[4];
    glm::vec3 m_vertexNormals[4];
    glm::vec3 m_vertexTangents[4];
    glm::vec2 m_vertexUVs[4];
    glm::ivec3 m_triangles[2];

    unsigned int m_numParticles = 0;

    void applyTranslation(const Vec3d t) override {};
    void applyRotation(const Mat3d r) override {};
    void applyScaling(const double s) override {};
    virtual void updatePostTransformData() override {};
};
}
