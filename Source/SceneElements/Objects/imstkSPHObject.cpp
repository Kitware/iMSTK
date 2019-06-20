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

#include "imstkSPHObject.h"

namespace imstk
{
SPHObject::SPHObject(const std::string& name) : DynamicObject<SPHKinematicState>(name)
{
    this->m_type = SceneObject::Type::SPH;
}

bool SPHObject::initialize()
{
    m_SPHModel = std::dynamic_pointer_cast<SPHModel>(this->m_dynamicalModel);
    if (m_SPHModel)
    {
        return m_SPHModel->initialize();
    }
    else
    {
        LOG(WARNING) << "Dynamics pointer cast failure in SPHObject::initialize()";
        return false;
    }
}
} // end namespace imstk
