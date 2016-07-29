#ifndef IMSTKPBDCOLLIDINGOBJECT_H
#define IMSTKPBDCOLLIDINGOBJECT_H

#include "imstkCollidingObject.h"

namespace imstk {

class PbdCollidingObject : public CollidingObject
{
protected:
    double m_proximity;
    double m_contactStiffness;
    std::vector<double> m_invMass;
public:
    PbdCollidingObject(std::string name)
        : CollidingObject(name)
    {

    }

    ~ PbdCollidingObject() = default;

    inline void setProximity(const double& prox)
    {
        m_proximity = prox;
    }

    inline double getProximity()
    {
        return m_proximity;
    }

    inline void setContactStiffness(const double& stiffness)
    {
        m_contactStiffness = stiffness;
    }

    inline double getContactStiffness()
    {
        return m_contactStiffness;
    }

    inline double getInvMass(const unsigned int& idx)
    {
        // should check bounds, but ignore for now due to bad design!
        return m_invMass[idx];
    }

    inline void setUniformMass(const double& val)
    {
        if (val >= 0)
        {
            std::fill(m_invMass.begin(), m_invMass.end(), val);
        }
    }

    inline void setFixedPoint(const unsigned int& idx)
    {
        // should check bounds, but ignore for now due to bad design!
        m_invMass[idx] = 0;
    }

    void setCollidingGeometry(std::shared_ptr<Geometry> geometry);

    Vec3d getVertexPosition(const unsigned int& idx);

    void setVertexPosition(const unsigned int& idx, const Vec3d& v);
};

}

#endif // IMSTKPBDCOLLIDINGOBJECT_H
