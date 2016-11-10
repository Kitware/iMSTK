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

#include "imstkDeformableObject.h"

namespace imstk
{

Vectord&
DeformableObject::getContactForce()
{
    //m_defModel = std::dynamic_pointer_cast<imstk::DeformableBodyModel>(m_dynamicalModel);

    if (!m_defModel)
    {
        LOG(WARNING) << "Dynamics pointer cast failure in DeformableObject::getContactForce()";
    }

    return m_defModel->getContactForce();
}

bool
DeformableObject::initialize()
{
    // CHECKBACK
    if (!m_dynamicalModel || m_dynamicalModel->getType() != DynamicalModelType::elastoDynamics)
    {
        LOG(WARNING) << "Dynamic model set is not of expected type (DeformableBodyModel)!";
        return false;
    }
    else
    {
        m_defModel = std::static_pointer_cast<DeformableBodyModel>(m_dynamicalModel);
        return true;
    }
}

const Vectord&
DeformableObject::getDisplacements() const
{
    return m_defModel->getCurrentState()->getQ();
}

const Vectord&
DeformableObject::getPrevDisplacements() const
{
    return m_defModel->getPreviousState()->getQ();
}

const Vectord&
DeformableObject::getVelocities() const
{
    return m_defModel->getCurrentState()->getQDot();
}

const Vectord&
DeformableObject::getPrevVelocities() const
{
    return m_defModel->getPreviousState()->getQDot();
}

const Vectord&
DeformableObject::getAccelerations() const
{
    return m_defModel->getCurrentState()->getQDotDot();
}

const Vectord&
DeformableObject::getPrevAccelerations() const
{
    return m_defModel->getPreviousState()->getQDotDot();
}

} // imstk
