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

#include <atomic>

namespace imstk
{
namespace ParallelUtils
{
///
/// \brief The ParallelSpinLock class
/// \brief A SpinLock is a light weight mutex,
/// which can be safely locked and unlocked exclusively by only one thread at a time
///
class ParallelSpinLock
{
public:
    ///
    /// \brief Constructor
    ///
    ParallelSpinLock() = default;

    ///
    /// \brief Copy constructor, must be implemented as an empty function
    /// because the member variable of type std::atomic_flag has copy constructor deleted
    ///
    ParallelSpinLock(const ParallelSpinLock&) {}

    ///
    /// \brief Start a thread-safe region, where only one thread can execute at a time until
    /// a call to the unlock function
    ///
    void lock()
    {
        while (m_Lock.test_and_set(std::memory_order_acquire)) {}
    }

    ///
    /// \brief End a thread-safe region
    ///
    void unlock()
    {
        m_Lock.clear(std::memory_order_release);
    }

private:
    std::atomic_flag m_Lock;
};
} // end namespace ParallelUtils
} // end namespace imstk
