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

#include "imstkDynamicObject.h"
#include "imstkDynamicalModel.h"
#include "imstkPbdModel.h"

#include <stdarg.h>

namespace imstk
{
class Geometry;
class GeometryMap;

///
/// \class PbdObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation
///
class PbdObject : public DynamicObject<PbdState>
{
public:
    ///
    /// \brief Constructor
    ///
    PbdObject(const std::string& name) : DynamicObject(name)
    {
        m_type = SceneObject::Type::Pbd;
    }

    PbdObject(std::string&& name) : DynamicObject(std::move(name))
    {
        m_type = SceneObject::Type::Pbd;
    }

    ///
    /// \brief Destructor
    ///
    virtual ~PbdObject() = default;

    ///
    /// \brief Initialize the pbd scene object
    ///
    bool initialize() override;

    ///
    /// \brief Update the position based on Verlet time stepping rule
    ///
    virtual void integratePosition();

    ///
    /// \brief Update the velocity
    ///
    virtual void updateVelocity();

    ///
    /// \brief Solve the pbd constraints by projection
    ///
    virtual void solveConstraints();

    ///
    /// \brief Reset the PBD object to its initial state
    ///
    void reset() override;

    void setPbdModel(std::shared_ptr<PbdModel> model) { m_pbdModel = model; }

protected:

    std::shared_ptr<PbdModel> m_pbdModel; ///> PBD mathematical model
};
} // imstk
