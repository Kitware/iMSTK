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

#pragma once

#include "imstkPbdConstraint.h"

#include <unordered_set>

namespace imstk
{
///
/// \class PbdConstraintContainer
///
/// \brief Container for pbd constraints
///
class PbdConstraintContainer
{
public:
    PbdConstraintContainer() = default;
    virtual ~PbdConstraintContainer() = default;

public:
    using iterator = std::vector<std::shared_ptr<PbdConstraint>>::iterator;

public:
    ///
    /// \brief Adds a constraint to the system, thread safe
    ///
    virtual void addConstraint(std::shared_ptr<PbdConstraint> constraint);

    ///
    /// \brief Linear searches for and removes a constraint from the system, thread safe
    ///
    virtual void removeConstraint(std::shared_ptr<PbdConstraint> constraint);

    ///
    /// \brief Removes all constraints associated with vertex ids
    ///
    virtual void removeConstraintVertices(std::shared_ptr<std::unordered_set<size_t>> vertices);

    ///
    /// \brief Removes a constraint from the system by iterator, thread safe
    ///
    virtual void eraseConstraint(iterator iter);

    ///
    /// \brief Reserve an amount of constraints in the pool, if you know
    /// ahead of time the number of constraints, or even an estimate, it
    /// can be faster to first reserve them
    ///
    virtual void reserve(const size_t n) { m_constraints.reserve(n); }

    ///
    /// \brief Returns if there are no constraints
    ///
    const bool empty() const { return m_constraints.empty() && m_partitionedConstraints.empty(); }

    ///
    /// \brief Get the underlying container
    ///
    const std::vector<std::shared_ptr<PbdConstraint>>& getConstraints() const { return m_constraints; }

    ///
    /// \brief Get the partitioned constraints
    ///
    const std::vector<std::vector<std::shared_ptr<PbdConstraint>>> getPartitionedConstraints() const { return m_partitionedConstraints; }

    ///
    /// \brief Partitions pbd constraints into separate vectors via graph coloring
    /// \param Minimum number of constraints in groups, any under will be dumped back into m_constraints
    ///
    void partitionConstraints(const int partitionThreshold);

    ///
    /// \brief Clear the parition vectors
    ///
    void clearPartitions() { m_partitionedConstraints.clear(); }

protected:
    std::vector<std::shared_ptr<PbdConstraint>> m_constraints;                         ///> Not partitioned constraints
    std::vector<std::vector<std::shared_ptr<PbdConstraint>>> m_partitionedConstraints; ///> Partitioned pbd constraints
    ParallelUtils::SpinLock m_constraintLock;                                          ///> Used to deal with concurrent addition/removal of constraints
};
}