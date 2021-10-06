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

#include <unordered_map>

namespace imstk
{
class EmbeddingConstraint;
class Geometry;
class PbdCollisionSolver;
class PbdCollisionConstraint;
}

using namespace imstk;

///
/// \brief Sorts and test equivalence of a set of vertex indices
///
struct TriCell
{
    std::uint32_t vertexIds[3];

    TriCell(std::uint32_t id0, std::uint32_t id1, std::uint32_t id2)
    {
        vertexIds[0] = id0;
        vertexIds[1] = id1;
        vertexIds[2] = id2;
        if (vertexIds[0] > vertexIds[1])
        {
            std::swap(vertexIds[0], vertexIds[1]);
        }
        if (vertexIds[1] > vertexIds[2])
        {
            std::swap(vertexIds[1], vertexIds[2]);
        }
        if (vertexIds[0] > vertexIds[1])
        {
            std::swap(vertexIds[0], vertexIds[1]);
        }
    }

    // Test true equivalence
    bool operator==(const TriCell& other) const
    {
        // Only works if sorted
        return (vertexIds[0] == other.vertexIds[0] && vertexIds[1] == other.vertexIds[1]
                && vertexIds[2] == other.vertexIds[2]);
    }
};
namespace std
{
template<>
struct hash<TriCell>
{
    // A 128 int could garuntee no collisions but its harder to find support for
    std::size_t operator()(const TriCell& k) const
    {
        using std::size_t;
        using std::hash;

        // Assuming sorted
        const std::size_t r =
            symCantor(static_cast<size_t>(k.vertexIds[0]), static_cast<size_t>(k.vertexIds[1]));
        return symCantor(r, static_cast<size_t>(k.vertexIds[2]));
    }
};
}

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
    virtual ~NeedleEmbeddedCH() override = default;

    virtual const std::string getTypeName() const override { return "NeedleEmbeddedCH"; }

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