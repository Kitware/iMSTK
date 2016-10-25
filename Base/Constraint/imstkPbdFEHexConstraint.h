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

#ifndef IMSTK_PBD_FE_HEX_CONSTRAINT_H
#define IMSTK_PBD_FE_HEX_CONSTRAINT_H

#include "imstkPbdFEMConstraint.h"

namespace imstk
{

///
/// \class FEMHexConstraint
///
/// \brief The FEMHexConstraint class class for constraint as the elastic energy
/// computed by linear shape functions with hexahedral mesh.
///
class FEMHexConstraint : public  FEMConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    explicit FEMHexConstraint(MaterialType mtype = MaterialType::StVK) :
        FEMConstraint(8, mtype) {}

    ///
    /// \brief Get the type of FEM constraint
    ///
    Type getType() const
    {
        return Type::FEMHex;
    }

    ///
    /// \brief Initializes the FEM hexahedral element constraint
    ///
    bool initConstraint(PositionBasedModel& model, const unsigned int& pIdx1,
        const unsigned int& pIdx2, const unsigned int& pIdx3,
        const unsigned int& pIdx4, const unsigned int& pIdx5,
        const unsigned int& pIdx6, const unsigned int& pIdx7,
        const unsigned int& pIdx8);

    ///
    /// \brief Solves the FEM hexahedral element constraint
    ///
    bool solvePositionConstraint(PositionBasedModel &model);
};

}

#endif // IMSTK_PBD_FE_HEX_CONSTRAINT_H