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
    void constraintProjection();

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

private:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<PbdState> m_state;
    std::vector<std::shared_ptr<PbdConstraint>> m_constraints;

    // Lame's constants
    double mu;
    double lambda;

    unsigned int maxIter; ///> Max. pbd iterations
    double m_proximity;
    double m_contactStiffness;
};

} // imstk

#endif // IMSTKPBDMODEL_H