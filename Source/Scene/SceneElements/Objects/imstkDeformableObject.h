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
#include <string>

// imstk
#include "imstkDynamicObject.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkMath.h"

namespace imstk
{
class Geometry;
class GeometryMap;

///
/// \class DeformableObject
///
/// \brief Scene objects that can deform
///
class FeDeformableObject : public DynamicObject
{
public:

    ///
    /// \brief Constructor
    ///
    explicit FeDeformableObject(const std::string& name) : DynamicObject(name) { m_type = Type::FEMDeformable; }
    FeDeformableObject() = delete;

    ///
    /// \brief Destructor
    ///
    ~FeDeformableObject() = default;

    ///
    /// \brief Initialize the deformable object
    ///
    bool initialize() override;

    ///
    /// \brief Initialize the kinematic state of the body
    ///
    void initializeState();
    void initializeState(const Vectord& p, const Vectord& v);

    ///
    /// \brief Set/Get dynamical model
    ///
    //void setDynamicalModel(std::shared_ptr<DynamicalModel<VectorizedState>> dynaDefModel) override;

    ///
    /// \brief Get the vector that holds the contact forces
    ///
    Vectord& getContactForce();

    ///
    ///  \brief Get the vector of current displacements
    ///
    const Vectord& getDisplacements() const;

    ///
    /// \brief Get the vector of displacements from previous time step
    ///
    const Vectord& getPrevDisplacements() const;

    ///
    /// \brief Get the vector of current velocities
    ///
    const Vectord& getVelocities() const;

    ///
    /// \brief Get the vector of velocities from previous time step
    ///
    const Vectord& getPrevVelocities() const;

    ///
    /// \brief Get the vector of current accelerations
    ///
    const Vectord& getAccelerations() const;

    ///
    /// \brief Get the vector of accelerations from previous time step
    ///
    const Vectord& getPrevAccelerations() const;

protected:
    std::shared_ptr<FEMDeformableBodyModel> m_defModel;
};
} // imstk
