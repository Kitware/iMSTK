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

#ifndef imstkRenderParticleEmitter_h
#define imstkRenderParticleEmitter_h

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

struct RenderParticleKeyFrame
{
    float m_time = 0;
    Color m_color = Color::White;
    Vec3f m_acceleration = Vec3f(0, 0, 0);
    float m_rotationalAcceleration = 0;
    float m_scale = 1.0f;
};

///
/// \class RenderParticleEmitter
///
/// \brief Particle emitter
///
class RenderParticleEmitter : public Geometry
{
public:
    ///
    /// \brief Shape of emitter
    ///
    enum class Shape
    {
        CUBE
    };

    ///
    /// \brief Mode of emitter
    ///
    enum class Mode
    {
        CONTINUOUS, ///< Emitter continuously releases/recycles particles
        BURST       ///< Emitter releases particles once until manually reset
    };

    ///
    /// \brief Constructor
    /// \param maxNumParticles Number of particles this emitter can produce
    /// \param time Lifespan of each particle (in milliseconds)
    /// \param mode Mode for emitter
    ///
    RenderParticleEmitter(unsigned int maxNumParticles = 128,
                          float time = 3000.0f,
                          Mode mode = Mode::CONTINUOUS);

    ///
    /// \brief Add keyframe to particle emitter
    /// \param keyFrame key frame to add
    /// \returns True if key frame added, false if too many key frames
    ///
    bool addKeyFrame(RenderParticleKeyFrame keyFrame);

    ///
    /// \brief Get mode of emitter
    /// \returns mode Mode of emitter
    ///
    RenderParticleEmitter::Mode getEmitterMode();

    ///
    /// \brief Set size of emitter
    /// \param size Width of emitter
    ///
    void setEmitterSize(float size);

    ///
    /// \brief Set size of particle
    /// \param size Particle size, this determines how much each keyframe
    ///        scales by
    ///
    void setParticleSize(float size);

    ///
    /// \brief Update function
    ///
    void updateParticleEmitter(Vec3d cameraPosition);

    ///
    /// \brief Emit particle
    ///
    void emitParticle(std::unique_ptr<RenderParticle>& particle);

    ///
    /// \brief Get number of particles
    ///
    unsigned int getNumParticles();

    ///
    /// \brief Get particles
    /// \returns particles
    ///
    std::vector<std::unique_ptr<RenderParticle>>& getParticles();

    ///
    /// \brief Get start and end frames
    ///
    RenderParticleKeyFrame * getStartKeyFrame();
    RenderParticleKeyFrame * getEndKeyFrame();

    ///
    /// \brief Get key frames
    /// \returns key frames that are unsorted
    ///
    std::vector<RenderParticleKeyFrame>& getKeyFrames();

    ///
    /// \brief Set velocity range
    /// This functions sets minimum and maximum rotation values for determining
    /// the initial trajectory of the particles. The values are randomly
    /// selected (according to a uniform distribution) between the min and max
    /// values. If the values are the same, then the particle direction will
    /// not behave randomly.
    /// \param minDirection Maximum initial angle of trajectory
    /// \param maxDirection Minimum initial angle of trajectory
    /// \param minSpeed Minimum initial speed
    /// \param maxSpeed Maximum initial speed
    /// \param minRotationSpeed Minimum initial rotation speed
    /// \param maxRotationSpeed Maximum initial rotation speed
    ///
    void setInitialVelocityRange(Vec3f minDirection,
                                 Vec3f maxDirection,
                                 float minSpeed,
                                 float maxSpeed,
                                 float minRotationSpeed,
                                 float maxRotationSpeed);

    ///
    /// \brief Get uniformly-distributed float
    /// \returns float in the range of [0, 1]
    ///
    float getRandomNormalizedFloat();

    ///
    /// \brief Get volume
    /// As these are particles, the volume is 0
    ///
    double getVolume() const override { return 0; };

    ///
    /// \brief Reset the emitter
    /// Only works for burst particles
    ///
    void reset();

protected:
    friend class VulkanParticleRenderDelegate;

    ///
    /// \brief Interpolate color
    ///
    void interpolateColor(Color& destination,
                          Color& sourceA,
                          Color& sourceB,
                          float alpha);

    ///
    /// \brief Initialize particles
    ///
    void initializeParticles();

    const int c_maxNumKeyFrames = 16; ///< Maximum key frames
    unsigned int m_maxNumParticles = 128; ///< Maximum particles

    RenderParticleEmitter::Mode m_mode
        = RenderParticleEmitter::Mode::CONTINUOUS;
    RenderParticleEmitter::Shape m_shape
        = RenderParticleEmitter::Shape::CUBE;
    float m_emitterSize = 1.0f;
    float m_particleSize = 0.1f;

    std::vector<std::unique_ptr<RenderParticle>> m_particles; ///< Particle objects
    std::vector<RenderParticleKeyFrame> m_keyFrames; ///< Particle keyframes
    imstk::StopWatch m_stopWatch;
    glm::vec3 m_vertexPositions[4];
    glm::vec3 m_vertexNormals[4];
    glm::vec3 m_vertexTangents[4];
    glm::vec2 m_vertexUVs[4];
    glm::ivec3 m_triangles[2];

    Vec3f m_minDirection;
    Vec3f m_maxDirection;
    float m_minSpeed;
    float m_maxSpeed;
    float m_minRotationSpeed;
    float m_maxRotationSpeed;

    void applyTranslation(const Vec3d t) override {};
    void applyRotation(const Mat3d r) override {};
    void applyScaling(const double s) override {};
    virtual void updatePostTransformData() override {};

    float m_time; ///< total time for particle system
    float m_emitTime;

    unsigned int m_numParticles = 0;
    double m_lastUpdateTime = 0.0;
    bool m_started = false;
};
}

#endif