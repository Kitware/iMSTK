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

#ifndef IMSTK_PBD_VOLUME_CONSTRAINT_H
#define IMSTK_PBD_VOLUME_CONSTRAINT_H

#include "imstkPbdConstraint.h"

namespace imstk
{

///
/// \class VolumeConstraint
///
/// \brief Volume constraint for tetrahedral element
///
class VolumeConstraint : public PbdConstraint
{
public:
    ///
    /// \brief constructor
    ///
    VolumeConstraint() : PbdConstraint(4) {}

    ///
    /// \brief Returns PBD constraint of type Type::Volume
    ///
    Type getType() const
    {
        return Type::Volume;
    }

    ///
    /// \brief Initializes the volume constraint
    ///
    void initConstraint(PositionBasedModel& model, const unsigned int& pIdx1, const unsigned int& pIdx2,
        const unsigned int& pIdx3, const unsigned int& pIdx4, const double k = 2.0);

    ///
    /// \brief Solves the volume constraint
    ///
    bool solvePositionConstraint(PositionBasedModel &model);

public:
    double m_restVolume; ///> Rest volume
    double m_stiffness;  ///> Stiffness of the volume constraint
};

}

#endif // IMSTK_PBD_DISTANCE_CONSTRAINT_H