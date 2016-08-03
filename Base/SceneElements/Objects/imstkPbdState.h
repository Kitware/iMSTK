#ifndef IMSTKPBDSTATE_H
#define IMSTKPBDSTATE_H

#include "imstkMesh.h"

#include <Eigen/Dense>
#include <vector>

namespace imstk
{

using Vec3d = Eigen::Vector3d;

class PbdState
{
private:

    std::vector<Vec3d> m_pos;
    std::vector<Vec3d> m_initPos;
    std::vector<Vec3d> m_vel;
    std::vector<Vec3d> m_acc;
    std::vector<Vec3d> m_oldPos;
    std::vector<double> m_mass;
    std::vector<double> m_invMass;
    double dt;
    Vec3d gravity;
public:
    PbdState() = default;

    void initialize(Mesh* m)
    {
        m_pos = m->getVerticesPositions(); // share the same data with Mesh
        const int nP =  m->getNumVertices();
        m_initPos.assign(m_pos.begin(), m_pos.end());
        m_vel.resize(nP, Vec3d(0,0,0));
        m_acc.resize(nP, Vec3d(0,0,0));
        m_oldPos.resize(nP, Vec3d(0,0,0));
        m_invMass.resize(nP, 0);
        m_mass.resize(nP, 0);
    }

    inline void setUniformMass(const double& val)
    {
        if (val != 0.0) {
            std::fill(m_mass.begin(), m_mass.end(), val);
            std::fill(m_invMass.begin(), m_invMass.end(), 1/val);
        }
        else {
            std::fill(m_invMass.begin(), m_invMass.end(), 0.0);
            std::fill(m_mass.begin(), m_mass.end(), 0.0);
        }
    }

    inline void setParticleMass(const double& val, const unsigned int& idx)
    {
        if ( idx < m_pos.size()) {
            m_mass[idx] = val;
            m_invMass[idx] = 1/val;
        }
    }

    inline void setFixedPoint(const unsigned int& idx)
    {        
        if ( idx < m_pos.size())
            m_invMass[idx] = 0;
    }

    inline double getInvMass(const unsigned int& idx)
    {
        return m_invMass[idx];
    }

    inline Vec3d& getInitialVertexPosition(const unsigned int& idx)
    {
        return m_initPos.at(idx);
    }

    inline Vec3d& getVertexPosition(const unsigned int& idx)
    {
        return m_pos.at(idx);
    }

    inline void setTimeStep(const double& timeStep) { dt = timeStep; }

    inline void setGravity(const Vec3d& g) { gravity = g; }

    void integratePosition();

    void integrateVelocity();

};

}


#endif // IMSTKPBDSTATE_H
