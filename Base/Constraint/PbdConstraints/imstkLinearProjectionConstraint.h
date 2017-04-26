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

#ifndef imstkLinearProjectionConstraint_h
#define imstkLinearProjectionConstraint_h

#include "imstkMath.h"

namespace imstk
{

///
/// \class LinearProjectionConstraint
///
/// \brief Linear projection constraint
///
class LinearProjectionConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    LinearProjectionConstraint(const size_t& nodeId, const bool isFixed = false);
    LinearProjectionConstraint() = delete;

    ///
    /// \brief Destructor
    ///
    ~LinearProjectionConstraint() = default;

    ///
    /// \brief Form the projection
    ///
    void setProjection(const size_t& nodeId, const Vec3d& p, const Vec3d& q = Vec3d::Zero());

    ///
    /// \brief Set the projector to simulate Dirichlet conditions
    ///
    void setProjectorToDirichlet(const size_t& nodeId);
    void setProjectorToDirichlet(const unsigned int& nodeId, const Vec3d z);

    ///
    /// \brief Reset the linear projector
    ///
    void reset();

    ///
    /// \brief Set the value in the restricted subspace
    ///
    inline void setValue(const Vec3d& v) { m_value = v; }

    ///
    /// \brief Get the projector
    ///
    inline const Mat3d& getProjector() const { return m_projection; }

    ///
    /// \brief Get the value
    ///
    inline const Vec3d& getValue() const { return m_value; }

    ///
    /// \brief Get the node id
    ///
    inline const size_t& getNodeId() const { return m_nodeId; }

    ///
    /// \brief Returns true if the constraint is fixed
    ///
    inline bool isFixed() const { return m_isFixedConstraint; }

private:
    size_t m_nodeId;                        ///> Node id
    bool m_isFixedConstraint = false;       ///> Flag to know if that node is fixed
    Mat3d m_projection = Mat3d::Identity(); ///> Orthogonal projector
    Vec3d m_value = Vec3d(0., 0., 0.);      ///> Value in the subspace: range(I-m_projector)
};

} // imstk

#endif // imstkLinearProjectionConstraint_h