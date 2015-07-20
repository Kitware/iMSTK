// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMPLANEMODEL_H
#define SMPLANEMODEL_H

// STL includes
#include <memory>
#include <array>

// Eigne includes
#include <Eigen/Geometry>

// SimMedTK includes
#include "Core/ModelRepresentation.h"
#include "Core/Geometry.h"

///
/// @brief Plane representation of a model.
/// Base class used by all models that can be represented by a plane
///
/// @see smPlaneCollisionModel
///
class smPlaneModel : public ModelRepresentation
{
public:
    using RigidTransformType = Eigen::Transform<double, 3, Eigen::Isometry>;

public:
    ///
    /// @brief Constructor
    ///
    smPlaneModel(const core::Vec3d& p, const core::Vec3d& n);

    ///
    /// @brief Destructor
    ///
    virtual ~smPlaneModel();

    ///
    /// @brief Draw this mesh
    ///
    void draw() override;

    ///
    /// @brief Returns normal vectors for triangles on mesh surface
    ///
    const core::Vec3d &getNormal() const;

    ///
    /// @brief Set plane normal
    ///
    void setNormal(const core::Vec3d &normal);

    ///
    /// @brief Get relative position of the plane
    ///
    const core::Vec3d &getPosition() const;

    ///
    /// @brief Get transformation operator
    ///
    const RigidTransformType &getTransform() const;

    ///
    /// @brief Set transformation operator
    ///
    void setTransform(const RigidTransformType &t);

    ///
    /// @brief Set transformation operator
    ///
    void setPlaneModel(const std::shared_ptr<Plane> &p);

    ///
    /// @brief Set transformation operator
    ///
    std::shared_ptr<Plane> getPlaneModel() const;

protected:
    // Plane data and transform
    std::shared_ptr<Plane> plane;
    RigidTransformType transform;
};

#endif // SMPLANEMODEL_H
