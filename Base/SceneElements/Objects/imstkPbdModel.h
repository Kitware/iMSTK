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

    inline void setNumberOfInterations(const unsigned int& n) { maxIter = n; }

    ///
    /// \brief create constraints from the underlying mesh structure
    /// \param type could be Distance,Dihedral, Area, Volume, FEMTet, FEMHex, EdgeEdge, PointTriangle
    ///
    bool initConstraints(PbdConstraint::Type type);
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

};

}




#endif // IMSTKPBDMODEL_H
