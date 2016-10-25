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

#ifndef IMSTK_PBD_FEM_CONSTRAINT_H
#define IMSTK_PBD_FEM_CONSTRAINT_H

#include "imstkPbdConstraint.h"

namespace imstk
{

///
/// \class FEMConstraint
///
/// \brief The FEMConstraint class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
/// We provide several model for elastic energy including:
/// Linear, Co-rotation, St Venant-Kirchhof and NeoHookean
///
class FEMConstraint : public PbdConstraint
{
public:
    // Material type
    enum class MaterialType
    {
        Linear,
        Corotation,
        StVK,
        NeoHookean
    };

    ///
    /// \brief
    ///
    explicit FEMConstraint(const unsigned int nP, MaterialType mtype = MaterialType::StVK) :
        PbdConstraint(nP), m_material(mtype) {}

public:
    double m_Volume;
    MaterialType m_material;
    Mat3d m_invRestMat;
};

}

#endif // IMSTK_PBD_FEM_CONSTRAINT_H