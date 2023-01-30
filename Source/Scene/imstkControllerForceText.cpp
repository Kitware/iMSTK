/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkControllerForceText.h"
#include "imstkGeometry.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkRenderMaterial.h"
#include "imstkSceneObject.h"
#include "imstkTextVisualModel.h"
#include "imstkVisualModel.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdContactConstraint.h"

namespace imstk
{
ControllerForceText::ControllerForceText(const std::string& name) : SceneBehaviour(name),
    m_textVisualModel(std::make_shared<TextVisualModel>("ControllerForceText"))
{
    m_textVisualModel->setPosition(TextVisualModel::DisplayPosition::UpperRight);
    m_textVisualModel->setFontSize(20.0);
}

void
ControllerForceText::init()
{
    // Add a visual representation for the object
    // how to avoid adding it twice?
    std::shared_ptr<Entity> entity = m_entity.lock();
    CHECK(entity != nullptr) << "ControllerForceText must have entity to initialize";
    if (!entity->containsComponent(m_textVisualModel))
    {
        m_textVisualModel->setName(entity->getName() + "_ControllerForceText");
        entity->addComponent(m_textVisualModel);
    }

    CHECK(m_pbdController != nullptr)
        << "ObjectControllerGhost must have a controller";
}

void
ControllerForceText::computePbdContactForceAndTorque(Vec3d& contactForce, Vec3d& contactTorque) const
{
    // \todo We should be able to do this with the PbdModel itself
    if (m_collision != nullptr)
    {
        auto                      pbdObj   = std::dynamic_pointer_cast<PbdObject>(m_pbdController->getControlledObject());
        std::shared_ptr<PbdModel> pbdModel = pbdObj->getPbdModel();
        const double              dt       = pbdModel->getConfig()->m_dt;
        const PbdParticleId       bodyId   = { pbdObj->getPbdBody()->bodyHandle, 0 };

        contactForce  = Vec3d::Zero();
        contactTorque = Vec3d::Zero();

        auto                               pbdCH = std::dynamic_pointer_cast<PbdCollisionHandling>(m_collision->getCollisionHandlingAB());
        const std::vector<PbdConstraint*>& collisionConstraints = pbdCH->getConstraints();
        for (const auto& constraint : collisionConstraints)
        {
            PbdContactConstraint* contactConstraint = dynamic_cast<PbdContactConstraint*>(constraint);
            if (contactConstraint == nullptr)
            {
                continue;
            }

            // Find the gradient of the constraint associated with the rigid body
            Vec3d grad = Vec3d::Zero();
            Vec3d r    = Vec3d::Zero();
            for (int i = 0; i < constraint->getParticles().size(); i++)
            {
                if (constraint->getParticles()[i] == bodyId)
                {
                    grad = constraint->getGradient(i);
                    r    = contactConstraint->getR(i);
                    break;
                }
            }

            // Multiply with gradient for direction
            const Vec3d force = constraint->getForce(dt) * grad;
            contactForce  += force;
            contactTorque += force.cross(r);
        }
    }
}

void
ControllerForceText::visualUpdate(const double& dt)
{
    // Only update when visible
    if (m_textVisualModel->getVisibility())
    {
        m_t += dt;

        // Only update every 0.1 sim time
        if (m_t > 0.1)
        {
            std::ostringstream strStream;
            strStream.precision(2);

            if (m_pbdController != nullptr)
            {
                const Vec3d  deviceForce  = m_pbdController->getDeviceForce();
                const Vec3d  deviceTorque = m_pbdController->getDeviceTorque();
                const double forceScaling = m_pbdController->getForceScaling();

                auto pbdObj = std::dynamic_pointer_cast<PbdObject>(m_pbdController->getControlledObject());

                // External/body force torque are cleared at the end of the frame so not possible to get here
                strStream <<
                    "Device Force: " << deviceForce.norm() << "N"
                    "\nDevice Torque: " << deviceTorque.norm() << "Nm";
                //"Body Force: " << pbdObj->getPbdBody()->externalForce.norm() << "N\n"
                //"Body Torque: " << pbdObj->getPbdBody()->externalTorque.norm() << "Nm\n";

                if (m_collision != nullptr)
                {
                    Vec3d contactForce  = Vec3d::Zero();
                    Vec3d contactTorque = Vec3d::Zero();
                    computePbdContactForceAndTorque(contactForce, contactTorque);

                    // Scale to bring in device space
                    const double contactForceMag  = contactForce.norm() * forceScaling;
                    const double contactTorqueMag = contactTorque.norm() * forceScaling;

                    strStream << "\nContact Force: " << contactForceMag << "N";
                    strStream << "\nContact Torque: " << contactTorqueMag << "Nm";
                }
            }

            m_textVisualModel->setText(strStream.str());
            m_t = 0.0;
        }
    }
}
} // namespace imstk
