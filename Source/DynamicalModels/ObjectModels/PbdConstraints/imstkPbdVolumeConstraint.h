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

#include "imstkPbdConstraint.h"

namespace imstk
{
///
/// \class VolumeConstraint
///
/// \brief Volume constraint for tetrahedral element
///
class PbdVolumeConstraint : public PbdConstraint
{
public:
    ///
    /// \brief constructor
    ///
    PbdVolumeConstraint() : PbdConstraint() { m_vertexIds.resize(4); }

    ///
    /// \brief Returns PBD constraint of type Type::Volume
    ///
    inline Type getType() const override { return Type::Volume; }

    ///
    /// \brief Initializes the volume constraint
    ///
    void initConstraint(PbdModel& model, const size_t& pIdx1,
                        const size_t& pIdx2, const size_t& pIdx3,
                        const size_t& pIdx4, const double k = 2.0);

    ///
    /// \brief Solves the volume constraint
    ///
    bool solvePositionConstraint(PbdModel& model) override;

public:
    double m_restVolume = 0.; ///> Rest volume
    double m_stiffness  = 0.; ///> Stiffness of the volume constraint
};
}
