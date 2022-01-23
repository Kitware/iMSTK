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

#include "imstkMath.h"
#include "imstkVecDataArray.h"

namespace imstk
{
///
/// \class PbdConstraint
///
/// \brief Base Constraint class for Position based dynamics constraints
///
class PbdConstraint
{
public:
    ///
    /// \brief Type of solvers
    ///
    enum class SolverType
    {
        xPBD = 0,
        PBD
    };

    PbdConstraint() = default;

    virtual ~PbdConstraint() = default;

    ///
    /// \brief abstract interface to know the type of constraint
    /// \return particular type
    ///
    virtual std::string getType() const = 0;

    ///
    /// \brief Compute value and gradient of the constraint
    ///
    /// \param[in] currVertexPositions vector of current positions
    /// \param[inout] c constraint value
    ///
    virtual bool computeValueAndGradient(
        const VecDataArray<double, 3>& currVertexPositions,
        double& c,
        std::vector<Vec3d>& dcdx) const = 0;

    ///
    /// \brief Get the vertex indices of the constraint
    ///
    std::vector<size_t>& getVertexIds() { return m_vertexIds; }

    ///
    /// \brief Set the tolerance used for pbd constraints
    ///
    void setTolerance(const double eps) { m_epsilon = eps; }

    ///
    /// \brief Get the tolerance used for pbd constraints
    ///
    double getTolerance() const { return m_epsilon; }

    ///
    /// \brief  Set the stiffness
    ///
    void setStiffness(const double stiffness)
    {
        m_stiffness  = stiffness;
        m_compliance = 1.0 / stiffness;
    }

    ///
    /// \brief  Get the stiffness
    ///
    double getStiffness() const { return m_stiffness; }

    ///
    /// \brief Use PBD
    ///
    void zeroOutLambda() { m_lambda = 0.0; }

    ///
    /// \brief Update positions by projecting constraints.
    ///
    virtual void projectConstraint(const DataArray<double>& currInvMasses, const double dt, const SolverType& type, VecDataArray<double, 3>& pos);

protected:
    std::vector<size_t> m_vertexIds;   ///> index of points for the constraint
    double m_epsilon        = 1.0e-16; ///> Tolerance used for the costraints
    double m_stiffness      = 1.0;     ///> used in PBD, [0, 1]
    double m_compliance     = 1e-7;    ///> used in xPBD, inverse of Young's Modulus
    mutable double m_lambda = 0.0;     ///> Lagrange multiplier

    std::vector<Vec3d> m_dcdx;
};
} // namespace imstk