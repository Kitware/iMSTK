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

#include <memory>
#include <vector>

#include "g3log/g3log.hpp"

#include "imstkMath.h"
#include "imstkColor.h"
#include "imstkTimer.h"
#include "imstkAnimationModel.h"
#include "imstkRenderParticles.h"

namespace imstk
{
///
/// \struct RenderParticleKeyFrame
///
/// \brief Keyframe for particle animation
///
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
/// \brief Animation method for rendering particles
/// Common use cases include smoke and fire.
///
class RenderParticleEmitter : public AnimationModel
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
    ///
    RenderParticleEmitter(std::shared_ptr<Geometry> geometry,
                          const float time = 3000.0f,
                          Mode mode = Mode::CONTINUOUS);

    ///
    /// \brief Set animation geometry
    /// \param renderParticles particles for rendering
    ///
    virtual void setGeometry(std::shared_ptr<Geometry> renderParticles);

    ///
    /// \brief Get mode of emitter
    /// \returns mode Mode of emitter
    ///
    RenderParticleEmitter::Mode getEmitterMode() const;

    ///
    /// \brief Set size of emitter
    /// \param size Width of emitter
    ///
    void setEmitterSize(const float size);

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
    void setInitialVelocityRange(const Vec3f minDirection,
                                 const Vec3f maxDirection,
                                 const float minSpeed,
                                 const float maxSpeed,
                                 const float minRotationSpeed,
                                 const float maxRotationSpeed);

    ///
    /// \brief Add keyframe to particle emitter
    /// \param keyFrame key frame to add
    /// \returns True if key frame added, false if too many key frames
    ///
    bool addKeyFrame(RenderParticleKeyFrame keyFrame);

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
    /// \brief Reset the emitter
    /// Only works for burst particles
    ///
    virtual void reset();

    ///
    /// \brief Update
    ///
    virtual void update();

protected:
    friend class VulkanParticleRenderDelegate;

    ///
    /// \brief Initialize particles
    ///
    void initializeParticles();

    ///
    /// \brief Interpolate color
    ///
    void interpolateColor(Color& destination,
                          const Color& sourceA,
                          const Color& sourceB,
                          const float alpha);

    ///
    /// \brief Emit particle
    ///
    void emitParticle(std::unique_ptr<RenderParticle>& particle);

    ///
    /// \brief Get uniformly-distributed float
    /// \returns float in the range of [0, 1]
    ///
    float getRandomNormalizedFloat();

    std::vector<RenderParticleKeyFrame> m_keyFrames; ///< Particle keyframes

    RenderParticleEmitter::Mode m_mode
        = RenderParticleEmitter::Mode::CONTINUOUS;
    RenderParticleEmitter::Shape m_shape
        = RenderParticleEmitter::Shape::CUBE;

    Vec3f m_minDirection;
    Vec3f m_maxDirection;
    float m_minSpeed;
    float m_maxSpeed;
    float m_minRotationSpeed;
    float m_maxRotationSpeed;

    float m_time; ///< total time for particle system
    float m_emitTime;

    imstk::StopWatch m_stopWatch;

    double m_lastUpdateTime = 0.0;
    bool m_started = false;

    float m_emitterSize = 1.0f;

    const int c_maxNumKeyFrames = 16; ///< Maximum key frames

    std::shared_ptr<Geometry> m_animationGeometry = nullptr;
    std::vector<std::unique_ptr<RenderParticle>> * m_particles;
};
} // imstk
