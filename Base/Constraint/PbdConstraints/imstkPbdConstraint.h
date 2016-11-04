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

const double EPS = 1e-6;

namespace imstk
{

class PositionBasedDynamicsModel;

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
        none
    };

    ///
    /// \brief Constructor
    ///
    PbdConstraint(const unsigned int nP) { m_vertexIds.resize(nP); }

    ///
    /// \brief abstract interface to know the type of constraint
    /// \return particular type
    ///
    virtual Type getType() const = 0;

    ///
    /// \brief update constraint
    /// \param model \class PositionBasedDynamicsModel
    /// \return true if succeeded
    ///
    virtual bool updateConstraint(PositionBasedDynamicsModel& model)
    {
        return true;
    }

    ///
    /// \brief compute delta position from the constraint function
    /// \param model \class PositionBasedDynamicsModel
    /// \return true if succeeded
    ///
    virtual bool solvePositionConstraint(PositionBasedDynamicsModel& model)
    {
        return true;
    }

    ///
    /// \brief compute delta velocity, specifically for rigid bodies
    /// \param model \class PositionBasedDynamicsModel
    /// \return true if succeeded
    ///
    virtual bool solveVelocityConstraint(PositionBasedDynamicsModel& model)
    {
        return true;
    }

public:
    std::vector<size_t> m_vertexIds; // index of points for the constraint
};

}

#endif // imstkPbdConstraint_h