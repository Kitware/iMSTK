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
#include "imstkRigidObject.h"
#include "imstkLogger.h"

namespace imstk
{
bool
RigidObject::initialize()
{
    auto m_rigidBodyModel = std::dynamic_pointer_cast<RigidBodyModel>(m_dynamicalModel);
    if (m_rigidBodyModel)
    {
        /*if (!m_rigidBodyModel->getRigidBodyWorld())
        {
            LOG(WARNING) << "RigidObject::initialize() - Rigid body world not specified!";
            return false;
        }*/
        return DynamicObject::initialize();
    }
    else
    {
        LOG(WARNING) << "RigidObject::initialize() - Dynamics pointer cast failure";
        return false;
    }
}
} // imstk
