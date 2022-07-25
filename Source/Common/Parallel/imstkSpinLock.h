/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <atomic>

namespace imstk
{
namespace ParallelUtils
{
///
/// \brief The SpinLock class
/// \brief A SpinLock is a light weight mutex,
/// which can be safely locked and unlocked exclusively by only one thread at a time
///
class SpinLock
{
public:
    ///
    /// \brief Default constructor, initializes the atomic_flag member to memory_order_release state
    ///
    SpinLock()
    {
        unlock();
    }

    ///
    /// \brief Copy constructor, must be implemented as an empty function
    /// because the member variable of type std::atomic_flag has copy constructor deleted.
    /// In addition, the constructor initializes the atomic_flag member to memory_order_release state
    ///
    SpinLock(const SpinLock&)
    {
        unlock();
    }

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
