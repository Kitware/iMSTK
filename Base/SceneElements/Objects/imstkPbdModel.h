#ifndef IMSTK_PBD_MODEL_H
#define IMSTK_PBD_MODEL_H

#include <vector>
#include <Eigen/Dense>

#include "imstkPbdConstraint.h"
#include "imstkPbdFEMConstraint.h"
#include "imstkPbdState.h"
#include "imstkMath.h"

namespace imstk
{

///
/// \class PositionBasedDynamicsModel
///
/// \brief This class implements position based dynamics mathematical model
///
class PositionBasedDynamicsModel
{
public:
    ///
    /// \brief
    ///
    PositionBasedDynamicsModel(){}

    ///
    /// \brief
    ///
    inline void setModelGeometry(const std::shared_ptr<Mesh>& m)
    {
        m_mesh = m;
        m_state = std::make_shared<PbdState>();
        m_state->initialize(m_mesh);

        auto nP = m_mesh->getNumVertices();
        m_invMass.resize(nP, 0);
        m_mass.resize(nP, 0);
    }

    ///
    /// \brief
    ///
    inline std::shared_ptr<Mesh> getGeometry()
    {
        return m_mesh;
    }

    ///
    /// \brief
    ///
    inline std::shared_ptr<PbdState> getState()
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
        m_mu = E/(2*(1+nu));
        m_lambda = E*nu/((1-2*nu)*(1+nu));
    }

    ///
    /// \brief
    ///
    inline const double& getFirstLame() const
    {
        return m_mu;
    }

    ///
    /// \brief
    ///
    inline const double& getSecondLame() const
    {
        return m_lambda;
    }

    ///
    /// \brief
    ///
    inline void setNumberOfInterations(const unsigned int& n)
    {
        m_maxIter = n;
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
    /// \brief
    ///
    bool initializeFEMConstraints(FEMConstraint::MaterialType type);

    ///
    /// \brief
    ///
    bool initializeVolumeConstraints(const double& stiffness);

    ///
    /// \brief
    ///
    bool initializeDistanceConstraints(const double& stiffness);

    ///
    /// \brief
    ///
    bool initializeAreaConstraints(const double& stiffness);

    ///
    /// \brief
    ///
    bool initializeDihedralConstraints(const double& stiffness);

    ///
    /// \brief addConstraint add elastic constraint
    /// \param constraint
    ///
    inline void addConstraint(std::shared_ptr<PbdConstraint> constraint)
    {
        m_constraints.push_back(constraint);
    }

    ///
    /// \brief compute delta x and update position
    ///
    void projectConstraints();

    ///
    /// \brief
    ///
    void updatePhysicsGeometry();

    ///
    /// \brief
    ///
    void updatePbdStateFromPhysicsGeometry();

    ///
    /// \brief
    ///
    inline bool hasConstraints() const
    {
        return !m_constraints.empty();
    }

    ///
    /// \brief
    ///
    void setTimeStep(const double& timeStep) { m_dt = timeStep; };

    ///
    /// \brief
    ///
    void setGravity(const Vec3d& g) { m_gravity = g; };

    ///
    /// \brief
    ///
    void setUniformMass(const double& val);

    ///
    /// \brief
    ///
    void setParticleMass(const double& val, const unsigned int& idx);

    ///
    /// \brief
    ///
    void setFixedPoint(const unsigned int& idx);

    ///
    /// \brief
    ///
    double getInvMass(const unsigned int& idx);

    ///
    /// \brief
    ///
    void integratePosition();

    ///
    /// \brief Integrate the velocity
    ///
    void integrateVelocity();

private:
    std::shared_ptr<Mesh> m_mesh;   ///> Mesh on which the pbd model operates on
    std::shared_ptr<PbdState> m_state;  ///> State of the body
    std::vector<std::shared_ptr<PbdConstraint>> m_constraints; ///> List of pbd constraints

    // Lame's constants
    double m_mu;
    double m_lambda;

    // Mass properties
    std::vector<double> m_mass;
    std::vector<double> m_invMass;

    double m_contactStiffness;
    Vec3d m_gravity;

    unsigned int m_maxIter; ///> Max. pbd iterations
    double m_proximity;

    double m_dt;
};

} // imstk

#endif // IMSTKPBDMODEL_H