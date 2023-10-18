/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDummyClient.h"
#include "imstkLogger.h"

namespace imstk
{
void
DummyClient::init(const unsigned int numButtons /*= 0*/)
{
    for (unsigned int i = 0; i < numButtons; i++)
    {
        m_buttons[i] = false;
    }
}

void
DummyClient::setPosition(const Vec3d& pos)
{
    m_transformLock.lock();
    m_position = pos;
    m_transformLock.unlock();
}

void
DummyClient::setVelocity(const Vec3d& vel)
{
    m_transformLock.lock();
    m_velocity = vel;
    m_transformLock.unlock();
}

void
DummyClient::setOrientation(const Quatd& orient)
{
    m_transformLock.lock();
    m_orientation = orient;
    m_transformLock.unlock();
}

void
DummyClient::setOrientation(double* transform)
{
    m_transformLock.lock();
    m_orientation = (Eigen::Affine3d(Eigen::Matrix4d(transform))).rotation();
    m_transformLock.unlock();
}

void
DummyClient::setButton(const unsigned int buttonId, const bool buttonStatus)
{
    m_dataLock.lock();
    auto x = m_buttons.find(buttonId);
    if (x != m_buttons.end())
    {
        x->second = buttonStatus;
    }
    m_dataLock.unlock();
}
} // namespace imstk