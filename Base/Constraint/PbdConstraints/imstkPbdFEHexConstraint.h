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

#ifndef imstkPbdFeHexConstraint_h
#define imstkPbdFeHexConstraint_h

#include "imstkPbdFEMConstraint.h"

namespace imstk
{

///
/// \class FEMHexConstraint
///
/// \brief The FEMHexConstraint class class for constraint as the elastic energy
/// computed by linear shape functions with hexahedral mesh.
///
class PbdFEMHexConstraint : public  PbdFEMConstraint
{
public:
    ///
    /// \brief Constructor
    ///
    explicit PbdFEMHexConstraint(MaterialType mtype = MaterialType::StVK) :
        PbdFEMConstraint(8, mtype) {}

    ///
    /// \brief Get the type of FEM constraint
    ///
    inline Type getType() const override { return Type::FEMHex; }

    ///
    /// \brief Initializes the FEM hexahedral element constraint
    ///
    bool initConstraint(PbdModel& model, const unsigned int& pIdx1,
        const unsigned int& pIdx2, const unsigned int& pIdx3,
        const unsigned int& pIdx4, const unsigned int& pIdx5,
        const unsigned int& pIdx6, const unsigned int& pIdx7,
        const unsigned int& pIdx8);

    ///
    /// \brief Solves the FEM hexahedral element constraint
    ///
    bool solvePositionConstraint(PbdModel &model) override;
};

} // imstk

#endif // imstkPbdFeHexConstraint_h