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

#include "imstkPbdFemConstraint.h"

namespace imstk
{
PbdFemConstraint::PbdFemConstraint(const unsigned int cardinality,
                                   MaterialType       mType /*= MaterialType::StVK*/) :
    PbdConstraint(),
    m_initialElementVolume(0),
    m_material(mType),
    m_invRestMat(Mat3d::Identity())
{
    m_vertexIds.resize(cardinality);
    m_dcdx.resize(cardinality);
}
} // namespace imstk