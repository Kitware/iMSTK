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

#include "imstkGeometry.h"
#include "imstkMath.h"
#include "imstkMacros.h"

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif

#ifdef WIN32
#pragma warning( pop )
#endif

namespace imstk
{
///
/// \struct RenderParticle
///
/// \brief Particle data
///
struct RenderParticle
{
    Vec3f m_position     = Vec3f(0, 0, 0);
    Vec3f m_velocity     = Vec3f(0, 0, 0);
    Vec3f m_acceleration = Vec3f(0, 0, 0);
    Vec4d m_color    = Vec4d(1., 1., 1., 1.);
    float m_age      = 0;
    bool m_created   = false;
    float m_scale    = 1.0f;
    float m_rotation = 0;
    float m_rotationalVelocity     = 0;
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
    ~RenderParticles() override = default;

    IMSTK_TYPE_NAME(RenderParticles)

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

protected:
    unsigned int m_numParticles    = 0;
    unsigned int m_maxNumParticles = 128; ///< Maximum particles
    float m_particleSize = 0.1f;

    std::vector<std::unique_ptr<RenderParticle>> m_particles; ///< Particle objects
    Vec3d m_vertexPositions[4];
    Vec3d m_vertexNormals[4];
    Vec3d m_vertexTangents[4];
    Vec2d m_vertexUVs[4];
    Vec3i m_triangles[2];

    void applyTransform(const Mat4d& m) override;

    void updatePostTransformData() const override {}
};
} // namespace imstk
