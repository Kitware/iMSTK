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

#include "imstkCollisionHandling.h"
#include "imstkSurfaceMesh.h"

#include <unordered_map>

namespace imstk
{
class EmbeddingConstraint;
class PbdCollisionSolver;
class PbdCollisionConstraint;
} // namespace imstk

using namespace imstk;

///
/// \class NeedleEmbeddedCH
///
/// \brief Implements PBD-RBD embedded tissue handling for when the
/// needle is embedded in the tissue
///
class NeedleEmbeddedCH : public CollisionHandling
{
public:
    NeedleEmbeddedCH();
    ~NeedleEmbeddedCH() override = default;

    const std::string getTypeName() const override { return "NeedleEmbeddedCH"; }

public:
    std::shared_ptr<Geometry> getHandlingGeometryA() override;

public:
    ///
    /// \brief Corrects for velocity (restitution and friction) after PBD is complete
    ///
    void correctVelocities();

    void solve();

    void setFriction(const double friction) { m_friction = friction; }
    const double getFriction() const { return m_friction; }

    void setCollisionSolver(std::shared_ptr<PbdCollisionSolver> solver) { m_solver = solver; }
    std::shared_ptr<PbdCollisionSolver> getCollisionSolver() const { return m_solver; }

protected:
    ///
    /// \brief Add embedding constraints based off contact data
    /// We need to add the constraint once and then update it later
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

private:
    // TriCell takes care of duplicate faces
    std::unordered_map<TriCell, std::shared_ptr<EmbeddingConstraint>> m_faceConstraints;

    std::shared_ptr<PbdCollisionSolver>  m_solver = nullptr;
    std::vector<PbdCollisionConstraint*> m_solverConstraints; ///> List of PBD constraints

    //double m_restitution = 0.0; ///> Coefficient of restitution (1.0 = perfect elastic, 0.0 = inelastic)
    double m_friction = 0.001; ///> Coefficient of friction (1.0 = full frictional force, 0.0 = none)

public:
    std::vector<Vec3d> m_debugEmbeddingPoints; ///> Used for debug visualization
    std::vector<Vec3i> m_debugEmbeddedTriangles;
};