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
#include "imstkSPHModel.h"

namespace imstk
{
bool
SPHObject::initialize()
{
    m_SPHModel = std::dynamic_pointer_cast<SPHModel>(m_dynamicalModel);
    if (m_SPHModel == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in SPHObject::initialize()";
        return false;
    }

    // why are we initializing twice?
    DynamicObject::initialize();
    m_SPHModel->initialize();

    return true;
}

std::shared_ptr<SPHModel>
SPHObject::getSPHModel()
{
    m_SPHModel = std::dynamic_pointer_cast<SPHModel>(m_dynamicalModel);
    return m_SPHModel;
}
} // end namespace imstk
