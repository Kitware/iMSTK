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
/// \class PBDModelConfig
/// \brief Parameters for PBD simulation
///
struct PBDModelConfig
{
    double m_uniformMassValue = 1.0;         ///> Mass properties
    double m_viscousDampingCoeff = 0.01;     ///> Viscous damping coefficient [0, 1]

    double m_contactStiffness = 1.0;         ///> Contact stiffness for collisions
    double m_proximity;                      ///> Proximity for collisions

    unsigned int m_maxIter;                 ///> Max. pbd iterations
    double m_dt;                            ///> Time step size
    double m_DefaultDt;                     ///> Default Time step size

    std::vector<std::size_t> m_fixedNodeIds; ///> Nodal IDs of the nodes that are fixed

    Vec3r m_gravity;                        ///> Gravity

    ////////////////////////////////////////////////////////////////////////////////
    struct Constraint
    {
        Constraint(PbdConstraint::Type type, double stiffness) :
            m_type(type), m_stiffness(stiffness) {}

        Constraint(PbdConstraint::Type type, PbdFEMConstraint::MaterialType FEMMaterial) :
            m_type(type), m_FEMMaterial(FEMMaterial) {}

        PbdConstraint::Type m_type;                   ///> Type of constraint
        double m_stiffness;                           ///> Constraint stiffness, if applicable
        PbdFEMConstraint::MaterialType m_FEMMaterial; ///> FEM material if constraint type is FEM
    };

    double m_mu;                                    ///> Lame constant, if constraint type is FEM
    double m_lambda;                                ///> Lame constant, if constraint type is FEM

    double m_YoungModulus;                          ///> FEM parameter, if constraint type is FEM
    double m_PoissonRatio;                          ///> FEM parameter, if constraint type is FEM

    std::vector<Constraint> m_constraints;
};

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
    PbdModel() : DynamicalModel(DynamicalModelType::positionBasedDynamics) {}

    ///
    /// \brief Destructor
    ///
    virtual ~PbdModel() override = default;

    ///
    /// \brief Set/Get the geometry (mesh in this case) used by the pbd model
    ///
    void setModelGeometry(const std::shared_ptr<PointSet>& m) { m_mesh = m; }
    const std::shared_ptr<PointSet>& getModelGeometry() const { return m_mesh; }

    ///
    /// \brief Set simulation parameters
    ///
    void configure(const std::shared_ptr<PBDModelConfig>& params);

    ///
    /// \brief Get the simulation parameters
    ///
    const std::shared_ptr<PBDModelConfig>& getParameters() const { assert(m_Parameters); return m_Parameters; }

    ///
    /// \brief Compute elastic constants: Young Modulus, Poisson Ratio, first and second Lame
    /// \brief If both Young Modulus or Poisson Ratio are zero, then compute them from the Lame coefficients
    /// \brief And vice versas, if both Lame coefficients are zero, compute them from Young Modulus and Poisson Ratio
    ///
    void computeElasticConstants();

    ///
    /// \brief Initialize FEM constraints
    ///
    bool initializeFEMConstraints(PbdFEMConstraint::MaterialType type);

    ///
    /// \brief Initialize volume constraints
    ///
    bool initializeVolumeConstraints(const double stiffness);

    ///
    /// \brief Initialize distance constraints
    ///
    bool initializeDistanceConstraints(const double stiffness);

    ///
    /// \brief Initialize area constraints
    ///
    bool initializeAreaConstraints(const double stiffness);

    ///
    /// \brief Initialize dihedral constraints
    ///
    bool initializeDihedralConstraints(const double stiffness);

    ///
    /// \brief addConstraint add elastic constraint
    /// \param constraint
    ///
    bool initializeConstantDensityConstraint(const double stiffness);

    ///
    /// \todo: add the initialization parameters for the constraint
    /// \param...
    ///

    inline void addConstraint(std::shared_ptr<PbdConstraint> constraint) { m_constraints.push_back(constraint); }

    ///
    /// \brief compute delta x (position) and update position
    ///
    void projectConstraints();

    ///
    /// \brief Update the model geometry from the newest PBD state
    ///
    void updatePhysicsGeometry() override;

    ///
    /// \brief Update the PBD state from the model geometry
    ///
    void updatePbdStateFromPhysicsGeometry();

    ///
    /// \brief Returns true if there is at least one constraint
    ///
    inline bool hasConstraints() const { return !m_constraints.empty(); }

    ///
    /// \brief Set the time step size
    ///
    virtual void setTimeStep(const Real timeStep) override { m_Parameters->m_dt = timeStep; }
    void setDefaultTimeStep(const Real timeStep) { m_Parameters->m_DefaultDt = static_cast<Real>(timeStep); }

    ///
    /// \brief Set the time step size to fixed size
    ///
    virtual void setTimeStepSizeType(const TimeSteppingType type) override;

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_Parameters->m_dt; }
    double getDefaultTimeStep() const { return m_Parameters->m_DefaultDt; }

    ///
    /// \brief Set uniform mass to all the nodes
    ///
    void setUniformMass(const double val);

    ///
    /// \brief Set mass to particular node
    ///
    void setParticleMass(const double val, const size_t idx);

    ///
    /// \brief Se the node as fixed
    ///
    void setFixedPoint(const size_t idx);

    ///
    /// \brief Get the inverse of mass of a certain node
    ///
    double getInvMass(const size_t idx) const;

    ///
    /// \brief Time integrate the position
    ///
    void integratePosition();

    ///
    /// \brief Time integrate the velocity
    ///
    void updateVelocity();

    ///
    /// \brief Update body states given the newest update and the type of update
    ///
    virtual void updateBodyStates(const Vectord& /*q*/,
                                  const stateUpdateType /*updateType = stateUpdateType::displacement*/) override {}

    ///
    /// \brief Initialize the PBD model
    ///
    bool initialize() override;

    ///
    /// \brief Return Constraints
    ///
    const std::vector<std::shared_ptr<PbdConstraint>> getConstraints() const { return m_constraints; }

protected:
    std::shared_ptr<PointSet> m_mesh;    ///> PointSet on which the pbd model operates on
    std::vector<double>       m_mass;    ///> Mass of nodes
    std::vector<double>       m_invMass; ///> Inverse of mass of nodes

    std::vector<std::shared_ptr<PbdConstraint>> m_constraints; ///> List of pbd constraints
    std::shared_ptr<PBDModelConfig>             m_Parameters;  ///> Model parameters, must be set before simulation
};
} // imstk

#endif // imstkPbdModel_h
