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

#ifndef IMSTK_PBD_FE_TET_CONSTRAINT_H
#define IMSTK_PBD_FE_TET_CONSTRAINT_H

#include "imstkPbdFEMConstraint.h"

namespace imstk
{

///
/// \class FEMTetConstraint
///
/// \brief The FEMTetConstraint class class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
///
class FEMTetConstraint : public  FEMConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    explicit FEMTetConstraint(MaterialType mtype = MaterialType::StVK) :
        FEMConstraint(4, mtype) {}

    ///
    /// \brief Get the type of FEM constraint
    ///
    Type getType() const
    {
        return Type::FEMTet;
    }

    ///
    /// \brief Initialize the tetrahedral FEM constraint
    ///
    bool initConstraint(PositionBasedModel& model, const unsigned int& pIdx1, const unsigned int& pIdx2,
        const unsigned int& pIdx3, const unsigned int& pIdx4);

    ///
    /// \brief Solve the tetrahedral FEM constraint
    ///
    bool solvePositionConstraint(PositionBasedModel &model);
};

}

#endif // IMSTK_PBD_FE_TET_CONSTRAINT_H