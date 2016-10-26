#ifndef IMSTK_PBD_STATE_H
#define IMSTK_PBD_STATE_H

#include <Eigen/Dense>
#include <vector>

#include "imstkProblemState.h"
#include "imstkMesh.h"
#include "imstkMath.h"

namespace imstk
{

///
/// \class PbdState
///
/// \brief State of the body governed by PBD mathematical model
///
class PbdState : public ProblemState
{
public:
    ///
    /// \brief Default constructor
    ///
    PbdState() = default;

    ///
    /// \brief
    ///
    void initialize(const std::shared_ptr<Mesh>& m);

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
    Vec3d& getInitialVertexPosition(const unsigned int& idx);

    ///
    /// \brief
    ///
    void setVertexPosition(const unsigned int& idx, Vec3d& pos);

    ///
    /// \brief
    ///
    Vec3d& getVertexPosition(const unsigned int& idx);

    ///
    /// \brief
    ///
    void setTimeStep(const double& timeStep) { dt = timeStep; };

    ///
    /// \brief
    ///
    void setGravity(const Vec3d& g) { gravity = g; };

    ///
    /// \brief
    ///
    void integratePosition();

    ///
    /// \brief
    ///
    void integrateVelocity();

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
};

} // imstk

#endif // IMSTK_PBD_STATE_H