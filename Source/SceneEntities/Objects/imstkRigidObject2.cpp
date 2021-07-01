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

#include "imstkRigidObject2.h"
#include "imstkLogger.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidBodyModel2.h"

namespace imstk
{
bool
RigidObject2::initialize()
{
    m_rigidBodyModel2 = std::dynamic_pointer_cast<RigidBodyModel2>(m_dynamicalModel);
    if (m_rigidBodyModel2 == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in RigidObject2::initialize()";
        return false;
    }

    DynamicObject::initialize();
    m_rigidBodyModel2->initialize();

    updateGeometries();

    return true;
}

std::shared_ptr<RigidBodyModel2>
RigidObject2::getRigidBodyModel2()
{
    m_rigidBodyModel2 = std::dynamic_pointer_cast<RigidBodyModel2>(m_dynamicalModel);
    return m_rigidBodyModel2;
}

void
RigidObject2::setDynamicalModel(std::shared_ptr<AbstractDynamicalModel> dynaModel)
{
    // todo: If already has another model, should remove the corresponding body?
    m_rigidBodyModel2 = std::dynamic_pointer_cast<RigidBodyModel2>(dynaModel);
    m_dynamicalModel  = dynaModel;
    m_rigidBody       = m_rigidBodyModel2->addRigidBody();
}

void
RigidObject2::updatePhysicsGeometry()
{
    // Apply the transform back to the geometry
    // If called before body is init'd use initial pose
    if (m_rigidBody->m_pos != nullptr)
    {
        m_physicsGeometry->setTranslation(m_rigidBody->getPosition());
    }
    else
    {
        m_physicsGeometry->setTranslation(m_rigidBody->m_initPos);
    }
    // If called before body is init'd use initial pose
    if (m_rigidBody->m_orientation != nullptr)
    {
        m_physicsGeometry->setRotation(m_rigidBody->getOrientation());
    }
    else
    {
        m_physicsGeometry->setRotation(m_rigidBody->m_initOrientation);
    }
    m_physicsGeometry->updatePostTransformData();

    DynamicObject::updatePhysicsGeometry();
}
} // imstk