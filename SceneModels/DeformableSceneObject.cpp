// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// iMSTK includes

#include "SceneModels/DeformableSceneObject.h"
#include "TimeIntegrators/BackwarEuler.h"
#include "TimeIntegrators/ForwardEuler.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"

#include <cmath> // for std::isfinite()

namespace imstk {

DeformableSceneObject::DeformableSceneObject():
    integrationScheme(TimeIntegrator::ImplicitEuler)
{
    this->gravity = Vec3d::UnitY();
}

//---------------------------------------------------------------------------
void DeformableSceneObject::applyContactForces()
{
    for(const auto & cf : this->getContactForces())
    {
        auto i = cf.first;
        this->f(i) -= cf.second(0);
        this->f(i + 1) -= cf.second(1);
        this->f(i + 2) -= cf.second(2);
    }
}

//---------------------------------------------------------------------------
void DeformableSceneObject::setTimeIntegrator(TimeIntegrator::IntegratorType integrator)
{
    this->integrationScheme = integrator;
}

//---------------------------------------------------------------------------
void DeformableSceneObject::initialize()
{
    switch(integrationScheme)
    {
        case TimeIntegrator::ImplicitEuler:
        {
            this->odeSolver = std::make_shared<BackwardEuler>(this);
            break;
        }

        case TimeIntegrator::ExplicitEuler:
        {
            this->odeSolver = std::make_shared<ForwardEuler>(this);
            break;
        }

        default:
        {
            // TODO: Log this
            std::cerr << "Invalid time integration scheme." << std::endl;
        }
    }
}

//---------------------------------------------------------------------------
void DeformableSceneObject::update(const double dt)
{
    if(!this->odeSolver)
    {
        std::cerr << "Ode solver needs to be set." << std::endl;
        return;
    }

    *this->newState = *this->currentState;

    this->odeSolver->solve(*this->currentState, *this->newState, dt);

    // Check state validity
    if(!std::isfinite(this->newState->getPositions().sum()) ||
       !std::isfinite(this->newState->getVelocities().sum()))
    {
        // TODO: log this and throw exception, this is a fatal error
        std::cerr << "Error: Invalid state." << std::endl;
        return;
    }

    this->currentState.swap(this->previousState);
    this->currentState.swap(this->newState);

    this->updateMesh();
}

//---------------------------------------------------------------------------
void DeformableSceneObject::resetToInitialState()
{
    *this->currentState = *this->initialState;
    *this->previousState = *this->initialState;
}

//---------------------------------------------------------------------------
std::shared_ptr< OdeSystemState > DeformableSceneObject::getCurrentState()
{
    return this->currentState;
}

//---------------------------------------------------------------------------
std::shared_ptr< OdeSystemState > DeformableSceneObject::getPreviousState()
{
    return this->previousState;
}

//---------------------------------------------------------------------------
Eigen::Map<Vec3d> DeformableSceneObject::getVelocity(const int index) const
{
    auto velocities = this->currentState->getVelocities();
    return Vec3d::Map(&velocities(index));
}

//---------------------------------------------------------------------------
const Vec3d& DeformableSceneObject::getGravity() const
{
    return this->gravity;
}

}
