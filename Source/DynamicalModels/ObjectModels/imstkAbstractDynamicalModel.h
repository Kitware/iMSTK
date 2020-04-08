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

#include "imstkVectorizedState.h"
#include "imstkGeometry.h"

#include <set>
#include <string>

namespace imstk
{
///
/// \brief Type of the time dependent mathematical model
///
enum class DynamicalModelType
{
    RigidBodyDynamics,
    ElastoDynamics,
    PositionBasedDynamics,
    SmoothedParticleHydrodynamics,
    None
};

///
/// \brief Type of the update of the state of the body
///
enum class TimeSteppingType
{
    RealTime,
    Fixed
};

///
/// \class DynamicalModel
///
/// \brief Abstract class for mathematical model of the physics governing the dynamic object
///
class AbstractDynamicalModel
{
public:

    ///
    /// \brief Type of the update of the state of the body
    ///
    enum class StateUpdateType
    {
        Displacement,
        Velocity,
        DeltaDisplacement,
        DeltaVelocity,
        None
    };

public:
    ///
    /// \brief Constructor
    ///
    AbstractDynamicalModel(DynamicalModelType type = DynamicalModelType::None) : m_type(type), m_numDOF(0) {}

    ///
    /// \brief Destructor
    ///
    virtual ~AbstractDynamicalModel() = default;

    ///
    /// \brief Reset the current state to the initial state
    ///
    virtual void resetToInitialState() = 0;

    ///
    /// \brief Returns the number of degrees of freedom
    ///
    std::size_t getNumDegreeOfFreedom() const { return m_numDOF; }
    void setNumDegreeOfFreedom(const size_t nDof) { m_numDOF = nDof; }

    ///
    /// \brief Get the type of the object
    ///
    const DynamicalModelType& getType() const { return m_type; }

    ///
    /// \brief Update states
    ///
    virtual void updateBodyStates(const Vectord& q, const StateUpdateType updateType = StateUpdateType::Displacement) = 0;

    ///
    /// \brief
    ///
    virtual void updatePhysicsGeometry() {}

    ///
    /// \brief Set the time step size
    ///
    virtual void setTimeStep(const double timeStep) = 0;

    ///
    /// \brief Sets the model geometry
    ///
    void setModelGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Checks if the given geometry is a valid geometry type for the model
    ///
    bool isGeometryValid(const std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Gets the model geometry
    ///
    std::shared_ptr<Geometry> getModelGeometry() const { return m_geometry; }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const = 0;

    ///
    /// \brief Initialize the dynamical model
    ///
    virtual bool initialize() = 0;

    ///
    /// \brief Set the type of approach used to update the time step size after every frame
    ///
    virtual void setTimeStepSizeType(const TimeSteppingType type) { m_timeStepSizeType = type; }
    TimeSteppingType getTimeStepSizeType() { return m_timeStepSizeType; }

protected:
    DynamicalModelType m_type;                      ///> Mathematical model type

    std::size_t m_numDOF;                           ///> Total number of degree of freedom

    std::shared_ptr<Geometry> m_geometry;           ///> Physics geometry of the model
    std::set<Geometry::Type>  m_validGeometryTypes; ///> Valid geometry types of this model

    TimeSteppingType m_timeStepSizeType = TimeSteppingType::Fixed;
};
} // imstk
