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
/// \struct PbdCollisionConstraintConfig
/// \brief Parameters for PBD collision constraints
///
struct PbdCollisionConstraintConfig
{
    PbdCollisionConstraintConfig(double proximity, double stiffness) : m_proximity(proximity), m_stiffness(stiffness) { }

    double m_proximity = 0.1; ///> Proximity for static collision
    double m_stiffness = 1.0; ///> Stiffness for collision
};

///
/// \class PbdCollisionConstraint
///
/// \brief
///
class PbdCollisionConstraint
{
public:
    enum class Type
    {
        EdgeEdge,
        PointTriangle,
        Analytical,
    };

    ///
    /// \brief
    ///
    PbdCollisionConstraint(const unsigned int& nA, const unsigned int& nB);

    ///
    /// \brief Destructor
    ///
    virtual ~PbdCollisionConstraint() = default;

    ///
    /// \brief Get vertex indices of first object
    ///
    const std::vector<size_t>& getVertexIdsFirst() const { return m_bodiesFirst; }
    const std::vector<size_t>& getVertexIdsSecond() const { return m_bodiesSecond; }

    ///
    /// \brief Get config
    ///
    const PbdCollisionConstraintConfig& getConfigFirst() const { return *m_configA; }
    const PbdCollisionConstraintConfig& getConfigSecond() const { return *m_configB; }

    ///
    /// \brief compute value and gradient of constraint function
    ///
    /// \param[in] currVertexPositionsA current positions from object A
    /// \param[in] currVertexPositionsA current positions from object B
    /// \param[inout] c constraint value
    /// \param[inout] dcdx constraint gradient
    ///
    virtual bool computeValueAndGradient(const VecDataArray<double, 3>& posA,
                                         const VecDataArray<double, 3>& posB,
                                         double& c,
                                         VecDataArray<double, 3>& dcdxA,
                                         VecDataArray<double, 3>& dcdxB) const = 0;

    virtual void projectConstraint(const DataArray<double>* invMassA,
                                   const DataArray<double>* invMassB,
                                   VecDataArray<double, 3>* posA,
                                   VecDataArray<double, 3>* posB);

protected:
    std::vector<size_t> m_bodiesFirst;                                 ///> index of points for the first object
    std::vector<size_t> m_bodiesSecond;                                ///> index of points for the second object

    std::shared_ptr<PbdCollisionConstraintConfig> m_configA = nullptr; ///> parameters of the collision constraint
    std::shared_ptr<PbdCollisionConstraintConfig> m_configB = nullptr; ///> parameters of the collision constraint
};

using PBDCollisionConstraintVector = std::vector<PbdCollisionConstraint*>;
}
