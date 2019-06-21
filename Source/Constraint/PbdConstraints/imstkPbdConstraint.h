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

#ifndef imstkPbdConstraint_h
#define imstkPbdConstraint_h

#include "imstkMath.h"

namespace imstk
{
class PbdModel;

///
/// \brief Base Constraint class for Position based dynamics constraints
///
class PbdConstraint
{
public:
    ///
    /// \brief Type of the PBD constraint
    ///
    enum class Type
    {
        Distance,
        Dihedral,
        Area,
        Volume,
        FEMTet,
        FEMHex,
        ConstantDensity,
        none
    };

    ///
    /// \brief Constructor
    ///
    PbdConstraint() = default;

    ///
    /// \brief Destructor
    ///
    virtual ~PbdConstraint() = default;

    ///
    /// \brief abstract interface to know the type of constraint
    /// \return particular type
    ///
    virtual Type getType() const = 0;

    ///
    /// \brief compute delta position from the constraint function
    /// \param model \class PbdModel
    /// \return true if succeeded
    ///
    virtual bool solvePositionConstraint(PbdModel& model) = 0;

    ///
    /// \brief Get the vertex indices of the constraint
    ///
    const std::vector<size_t>& getVertexIds() const { return m_vertexIds; }

    ///
    /// \brief Set the tolerance used for pbd constraints
    ///
    void setTolerance(const double eps) { m_epsilon = eps; }

protected:
    std::vector<size_t> m_vertexIds;   ///> index of points for the constraint
    double m_epsilon = 1.0e-6;         ///> Tolerance used for the costraints
};
}

#endif // imstkPbdConstraint_h
