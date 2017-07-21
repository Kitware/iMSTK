/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#ifndef imstkPbdModel_h
#define imstkPbdModel_h

#include <vector>
#include <Eigen/Dense>

#include "imstkPbdConstraint.h"
#include "imstkPbdFEMConstraint.h"
#include "imstkDynamicalModel.h"
#include "imstkPbdState.h"
#include "imstkMath.h"

namespace imstk
{
///
/// \class PbdModel
///
/// \brief This class implements position based dynamics mathematical model
///
class PbdModel : public DynamicalModel<PbdState>
{
public:
    ///
    /// \brief Constructor
    ///
    PbdModel();

    ///
    /// \brief Destructor
    ///
    ~PbdModel() = default;

    ///
    /// \brief Initialize the states
    ///
    bool initialize();

    ///
    /// \brief Set/Get the geometry (mesh in this case) used by the pbd model
    ///
    void setModelGeometry(std::shared_ptr<Mesh> m);
    std::shared_ptr<Mesh> getModelGeometry() const { return m_mesh; }

    ///
    /// \brief setElasticModulus
    /// \param E  Young's modulus
    /// \param nu Poisson's ratio
    ///
    void computeLameConstants(const double& E, const double nu);

    ///
    /// \brief Returns the first Lame constant
    ///
    const double& getFirstLame() const { return m_mu; }

    ///
    /// \brief Returns the second Lame constant
    ///
    const double& getSecondLame() const { return m_lambda; }

    ///
    /// \brief Set the maximum number of iterations for the pbd solver
    ///
    void setMaxNumIterations(const unsigned int& n) { m_maxIter = n; }

    ///
    /// \brief Get/Set proximity used for collision
    ///
    void setProximity(const double& prox) { m_proximity = prox; }
    double getProximity() const { return m_proximity; }

    ///
    /// \brief Get/Set contact stiffness that is used for collision constraints
    ///
    void setContactStiffness(const double& stiffness) { m_contactStiffness = stiffness;}
    double getContactStiffness() const { return m_contactStiffness; }

    ///
    /// \brief Initialize FEM constraints
    ///
    bool initializeFEMConstraints(PbdFEMConstraint::MaterialType type);

    ///
    /// \brief Initialize volume constraints
    ///
    bool initializeVolumeConstraints(const double& stiffness);

    ///
    /// \brief Initialize distance constraints
    ///
    bool initializeDistanceConstraints(const double& stiffness);

    ///
    /// \brief Initialize area constraints
    ///
    bool initializeAreaConstraints(const double& stiffness);

    ///
    /// \brief Initialize dihedral constraints
    ///
    bool initializeDihedralConstraints(const double& stiffness);

    ///
    /// \brief addConstraint add elastic constraint
    /// \param constraint
    ///
    bool initializeConstantDensityConstraint(const double& stiffness);

    ///
    /// \todo: add the initialization parameters for the constraint
    /// \param...
    ///

    inline void addConstraint(std::shared_ptr<PbdConstraint> constraint) { m_constraints.push_back(constraint); }

    ///
    /// \brief compute delta x and update position
    ///
    void projectConstraints();

    ///
    /// \brief Update the model geometry from the newest pbd state
    ///
    void updatePhysicsGeometry() override;

    ///
    /// \brief Update the pbd state from the model geometry
    ///
    void updatePbdStateFromPhysicsGeometry();

    ///
    /// \brief Returns true if there is atleast one constraint
    ///
    inline bool hasConstraints() const { return !m_constraints.empty(); }

    ///
    /// \brief Set the time step size
    ///
    void setTimeStep(const double& timeStep) { m_dt = timeStep; };

    ///
    /// \brief Returns the time step size
    ///
    double getTimeStep() const { return m_dt; };

    ///
    /// \brief Set the gravity
    ///
    void setGravity(const Vec3d& g) { m_gravity = g; };

    ///
    /// \brief Set/get viscous damping coefficient. Will be applied globally
    ///
    void setViscousDamping(const double damping);
    double getViscousDamping() {return m_viscousDampingCoeff; }

    ///
    /// \brief Set uniform mass to all the nodes
    ///
    void setUniformMass(const double& val);

    ///
    /// \brief Set mass to particular node
    ///
    void setParticleMass(const double& val, const size_t& idx);

    ///
    /// \brief Se the node as fixed
    ///
    void setFixedPoint(const size_t& idx);

    ///
    /// \brief Get the inverse of mass of a certain node
    ///
    double getInvMass(const size_t& idx) const;

    ///
    /// \brief Time integrate the position
    ///
    void integratePosition();

    ///
    /// \brief Time integrate the velocity
    ///
    void updateVelocity();

    ///
    /// \brief
    ///
    void updateBodyStates(const Vectord& q, const stateUpdateType updateType = stateUpdateType::displacement) override {};

protected:
    std::shared_ptr<Mesh> m_mesh;   ///> Mesh on which the pbd model operates on
    std::vector<std::shared_ptr<PbdConstraint>> m_constraints; ///> List of pbd constraints

    // Lame's constants
    double m_mu;                        ///> Lame constant
    double m_lambda;                    ///> Lame constant

    // Mass properties
    std::vector<double> m_mass;         ///> Mass of nodes
    std::vector<double> m_invMass;      ///> Inverse of mass of nodes

    double m_contactStiffness;            ///> Contact stiffness for collisions
    Vec3d m_gravity;                      ///> Gravity

    double m_viscousDampingCoeff = 0.01;  ///> Viscous damping coefficient [0, 1]

    unsigned int m_maxIter;               ///> Max. pbd iterations
    double m_proximity;                   ///> Proximity for collisions

    double m_dt;                          ///> Time step size
};
} // imstk

#endif // imstkPbdModel_h