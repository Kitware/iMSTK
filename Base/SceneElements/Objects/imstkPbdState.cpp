#include "imstkPbdState.h"

namespace imstk
{

void
PbdState::integratePosition()
{
    for (int i = 0; i < m_pos.size(); ++i)
    {
       if (m_invMass[i] !=  0.0)
       {
           m_vel[i] += (m_acc[i] + gravity)*dt;
           m_oldPos[i] = m_pos[i];
           m_pos[i] += m_vel[i]*dt;

       }
    }
}

void
PbdState::integrateVelocity()
{
    for (int i = 0; i < m_pos.size(); ++i)
    {
       if (m_invMass[i] != 0.0)
       {
           m_vel[i] = (m_pos[i] - m_oldPos[i])/dt;
       }
    }
}

} // imstk