#ifndef IMSTKPBDMODEL_H
#define IMSTKPBDMODEL_H

#include <vector>
#include <Eigen/Dense>

#include "imstkPbdConstraint.h"
#include "imstkPbdState.h"

namespace imstk
{

using Vec3d = Eigen::Vector3d;
///
/// \brief The PositionBasedModel class
///
class PositionBasedModel
{
private:
    Mesh*                         m_mesh;
    PbdState*                     m_state;
    std::vector<PbdConstraint*>   m_constraints;
    double mu, lambda;            // Lame's constants
    unsigned int maxIter;
    double m_proximity;
    double m_contactStiffness;
public:
    PositionBasedModel()
    {

    }

    inline void setModelGeometry(Mesh* m)
    {
        m_mesh = m;
        m_state = new PbdState;
        m_state->initialize(m_mesh);
    }

    inline Mesh* getGeometry()
    {
        return m_mesh;
    }

    inline PbdState* getState()
    {
        return m_state;
    }
    ///
    /// \brief setElasticModulus
    /// \param E  Young's modulus
    /// \param nu Poisson's ratio
    ///
    inline void setElasticModulus(const double& E, const double nu)
    {
        mu = E/(2*(1+nu));
        lambda = E*nu/((1-2*nu)*(1+nu));
    }

    inline const double& getFirstLame() const
    {
        return mu;
    }

    inline const double& getSecondLame() const
    {
        return lambda;
    }

    inline void setNumberOfInterations(const unsigned int& n)
    {
        maxIter = n;
    }

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

    ///
    /// \brief create constraints from the underlying mesh structure
    ///
    bool initFEMConstraints(FEMConstraint::MaterialType type);
    bool initVolumeConstraints(const double& stiffness);
    bool initDistanceConstraints(const double& stiffness);
    bool initAreaConstraints(const double& stiffness);
    bool initDihedralConstraints(const double& stiffness);
    ///
    /// \brief addConstraint add elastic constraint
    /// \param constraint
    ///
    inline void addConstraint(PbdConstraint* constraint)
    {
        m_constraints.push_back(constraint);
    }

    ///
    /// \brief compute delta x and update position
    ///
    void constraintProjection();

    void updatePhysicsGeometry();

    inline bool hasConstraints() const
    {
        return !m_constraints.empty();
    }

};

}




#endif // IMSTKPBDMODEL_H
