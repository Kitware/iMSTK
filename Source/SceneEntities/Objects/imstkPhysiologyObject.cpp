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

#include "imstkPhysiologyObject.h"
#include "imstkPhysiologyModel.h"

namespace imstk
{
  PhysiologyObject::PhysiologyObject(const std::string& name) : DynamicObject(name)
{
    this->m_type = Type::Physiology;
}

std::shared_ptr<PhysiologyModel> PhysiologyObject::getPhysiologyModel()
{
    m_PhysiologyModel = std::dynamic_pointer_cast<PhysiologyModel>(m_dynamicalModel);
    return m_PhysiologyModel;
};

bool
PhysiologyObject::initialize()
{
    m_PhysiologyModel = std::dynamic_pointer_cast<PhysiologyModel>(m_dynamicalModel);
    if (m_PhysiologyModel == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in SPHObject::initialize()";
        return false;
    }

    DynamicObject::initialize();
    m_PhysiologyModel->initialize();

    return true;
}
} // end namespace imstk
