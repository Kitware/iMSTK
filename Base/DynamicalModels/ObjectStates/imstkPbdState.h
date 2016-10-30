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

#ifndef imstkPbdState_h
#define imstkPbdState_h

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
    /// \brief Default constructor/destructor
    ///
    PbdState() = default;
    ~PbdState() = default;

    ///
    /// \brief
    ///
    void initialize(const size_t numNodes, bool p=1, bool v=1, bool a=1);
    void initialize(const std::shared_ptr<Mesh>& m, bool p=1, bool v=1, bool a=1);

    ///
    /// \brief
    ///
    void setVertexPosition(const unsigned int& idx, Vec3d& pos) { m_pos.at(idx) = pos; };
    Vec3d& getVertexPosition(const unsigned int& idx) { return m_pos.at(idx); };

    ///
    /// \brief
    ///
    std::vector<Vec3d>& getPositions() { return m_pos; };
    void setPositions(const std::vector<Vec3d>& p) { m_pos = p; };

    ///
    /// \brief
    ///
    std::vector<Vec3d>& getVelocities() { return m_vel; };

    ///
    /// \brief
    ///
    std::vector<Vec3d>& getAccelerations() { return m_acc; };

private:
    std::vector<Vec3d> m_pos; ///> Positions
    std::vector<Vec3d> m_vel; ///> Velocities
    std::vector<Vec3d> m_acc; ///> Acelerations
};

} // imstk

#endif // IMSTK_PBD_STATE_H