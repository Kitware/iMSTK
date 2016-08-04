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
/// \class PositionBasedModel
///
/// \brief
///
class PositionBasedModel
{
public:
    ///
    /// \brief
    ///
    PositionBasedModel()
    {

    }

    ///
    /// \brief
    ///
    inline void setModelGeometry(Mesh* m)
    {
        m_mesh = m;
        m_state = new PbdState;
        m_state->initialize(m_mesh);
    }

    ///
    /// \brief
    ///
    inline Mesh* getGeometry()
    {
        return m_mesh;
    }

    ///
    /// \brief
    ///
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

    ///
    /// \brief
    ///
    inline const double& getFirstLame() const
    {
        return mu;
    }

    ///
    /// \brief
    ///
    inline const double& getSecondLame() const
    {
        return lambda;
    }

    ///
    /// \brief
    ///
    inline void setNumberOfInterations(const unsigned int& n)
    {
        maxIter = n;
    }

    ///
    /// \brief
    ///
    inline void setProximity(const double& prox)
    {
        m_proximity = prox;
    }

    ///
    /// \brief
    ///
    inline double getProximity()
    {
        return m_proximity;
    }

    ///
    /// \brief
    ///
    inline void setContactStiffness(const double& stiffness)
    {
        m_contactStiffness = stiffness;
    }

    ///
    /// \brief
    ///
    inline double getContactStiffness()
    {
        return m_contactStiffness;
    }

    ///
    /// \brief create constraints from the underlying mesh structure
    ///
    bool initFEMConstraints(FEMConstraint::MaterialType type);

    ///
    /// \brief
    ///
    bool initVolumeConstraints(const double& stiffness);

    ///
    /// \brief
    ///
    bool initDistanceConstraints(const double& stiffness);

    ///
    /// \brief
    ///
    bool initAreaConstraints(const double& stiffness);

    ///
    /// \brief
    ///
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

    ///
    /// \brief
    ///
    void updatePhysicsGeometry();

    ///
    /// \brief
    ///
    inline bool hasConstraints() const
    {
        return !m_constraints.empty();
    }

private:
    Mesh*                         m_mesh;
    PbdState*                     m_state;
    std::vector<PbdConstraint*>   m_constraints;
    double mu, lambda;            // Lame's constants
    unsigned int maxIter;
    double m_proximity;
    double m_contactStiffness;
};

} // imstk

#endif // IMSTKPBDMODEL_H