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

#include "imstkPbdCollisionConstraint.h"
#include "imstkRbdConstraint.h"

namespace imstk
{
///
/// \brief Given a line and a triangle it computes the intersection between
/// them upon initialation for which it saves the interpolation weights.
///
/// This allows us to deform both shapes whilst maintaining a relative
/// position on that element.
///
/// To then constrain we compute the difference between the two interpolated
/// positions on each element and pull the line back towards the triangle
/// via rbd constraint and pull the triangle back towards the line via pbd
///
/// A compliance term gives the weighting for which to do this. To make more
/// physically accurate we would need to take an approach like XPBD in solving.
///
class EmbeddingConstraint : public PbdCollisionConstraint, public RbdConstraint
{
public:
    EmbeddingConstraint(std::shared_ptr<RigidBody> obj1) :
        PbdCollisionConstraint(1, 3), RbdConstraint(obj1, nullptr, RbdConstraint::Side::A)
    {
    }

    ~EmbeddingConstraint() override = default;

public:
    ///
    /// \brief Initializes both PBD and RBD constraint
    ///
    void initConstraint(
        VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
        Vec3d* p, Vec3d* q);

public:
    ///
    /// \brief Given two interpolants on the two elements, compute the difference
    /// between them and use for resolution
    ///
    Vec3d computeInterpolantDifference() const;

public:
    ///
    /// \brief Update the pbd constraint
    ///
    bool computeValueAndGradient(double&             c,
                                 std::vector<Vec3d>& dcdxA,
                                 std::vector<Vec3d>& dcdxB) const override;

public:
    ///
    /// \brief Update the rbd constraint
    ///
    void compute(double dt) override;

protected:
    // Intersection point via interpolants on triangle
    Vec3d m_uvw = Vec3d::Zero();
    // Intersection point via interpolants on line
    double m_t = 0.0;

    // The actual line
    Vec3d* m_p = nullptr;
    Vec3d* m_q = nullptr;

// The triangle is stored in PBD base class

protected:
    // Step for rbd constraint
    double m_beta = 0.05;

    Vec3d m_iPt;
    Vec3d m_iPtVel;

protected:
    // Ratio between the two models (ie: how much rbd tool is moved vs how much pbd tissue is)
    //
    // If 0.0, rbd tool is completely resolved and PBD tissue does not move
    // If 1.0, pbd tissue completely moves and rbd tool feels no resistance
    double m_compliance = 0.5;

    //// Ratio between the two models. This gives how much the rbd tool is moved vs how much pbd tissue is
    //// in the normal direction
    //double m_normalCompliance = 0.5;

    // If 0.0, completely removes pbd reaction in line axes direction, the pbd triangle will completely let
    // the tool slide in that direction
    // If 1.0, completely resist normal movement
    double m_normalFriction = 0.0;
};
}