#include "imstkPbdState.h"

namespace imstk {

void PbdState::computeAABB(double &min_x, double &max_x, double &min_y, double &max_y, double &min_z, double &max_z)
{
    min_x = 1e6;
    max_x = -1e6;
    min_y = 1e6;
    max_y = -1e6;
    min_z = 1e6;
    max_z = -1e6;
    for (int i = 0; i < m_pos.size(); ++i) {
       Vec3d& p = m_pos[i];
       if (p[0] < min_x) min_x = p[0];
       if (p[0] > max_x) max_x = p[0];
       if (p[1] < min_y) min_y = p[1];
       if (p[1] > max_y) max_y = p[1];
       if (p[2] < min_z) min_z = p[2];
       if (p[2] > max_z) max_z = p[2];
    }
}

void PbdState::integratePosition()
{
    for (int i = 0; i < m_pos.size(); ++i) {
       if (m_invMass[i] > 0) {
           m_vel[i] += (m_acc[i] + gravity)*dt;
           m_oldPos[i] = m_pos[i];
           m_pos[i] += m_vel[i]*dt;
       }
    }
}

void PbdState::integrateVelocity()
{
    for (int i = 0; i < m_pos.size(); ++i) {
       if (m_invMass[i] > 0) {
           m_vel[i] = (m_pos[i] - m_oldPos[i])/dt;
       }
    }
}

}
