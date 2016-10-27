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
    Vec3d& getInitialVertexPosition(const unsigned int& idx);

    ///
    /// \brief
    ///
    void setVertexPosition(const unsigned int& idx, Vec3d& pos);

    ///
    /// \brief
    ///
    Vec3d& getVertexPosition(const unsigned int& idx);

    std::vector<Vec3d>& getInitPositions() { return m_initPos; };
    std::vector<Vec3d>& getPreviousPositions() { return m_oldPos; };
    std::vector<Vec3d>& getPositions() { return m_pos; };
    std::vector<Vec3d>& getVelocities() { return m_vel; };
    std::vector<Vec3d>& getAccelerations() { return m_acc; };

private:
    std::vector<Vec3d> m_pos;
    std::vector<Vec3d> m_initPos;
    std::vector<Vec3d> m_vel;
    std::vector<Vec3d> m_acc;
    std::vector<Vec3d> m_oldPos;
};

} // imstk

#endif // IMSTK_PBD_STATE_H