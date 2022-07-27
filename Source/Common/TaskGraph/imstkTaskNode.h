/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <atomic>
#include <functional>
#include <string>

namespace imstk
{
///
/// \class TaskNode
///
/// \brief Base class for TaskGraph nodes
///
class TaskNode
{
public:
    TaskNode() : m_globalId(TaskNode::getUniqueID()) { }

    TaskNode(std::function<void()> func, std::string name = "none", bool isCritical = false) :
        m_name(name), m_isCritical(isCritical),
        m_computeTime(0.0), m_func(func),
        m_globalId(TaskNode::getUniqueID())
    {
    }

    TaskNode(TaskNode& other)
    {
        // When copied from another take all tis values
        // but its global id
        m_name         = other.m_name;
        m_enabled      = other.m_enabled;
        m_isCritical   = other.m_isCritical;
        m_computeTime  = other.m_computeTime;
        m_enableTiming = other.m_enableTiming;
        m_func         = other.m_func;
        m_globalId     = getUniqueID();
    }

    void operator=(const TaskNode& other)
    {
        // When set equal to another take all its values but
        // its global id
        m_name         = other.m_name;
        m_enabled      = other.m_enabled;
        m_isCritical   = other.m_isCritical;
        m_computeTime  = other.m_computeTime;
        m_enableTiming = other.m_enableTiming;
        m_func         = other.m_func;
        m_globalId     = getUniqueID();
    }

    virtual ~TaskNode() = default;

    void setFunction(std::function<void()> func) { this->m_func = func; }
    void setEnabled(bool enabled) { this->m_enabled = enabled; }

    ///
    /// \brief Returns true if function is nullptr
    ///
    bool isFunctional() const { return m_func != nullptr; }

    ///
    /// \brief Calls the function pointer provided if node enabled
    ///
    virtual void execute();

    ///
    /// \brief Get the global (unique) index of the geometry
    ///
    size_t getGlobalId() const { return m_globalId; }

    ///
    /// \brief Get number of ids/taskNodes
    ///
    static size_t getNumGlobalIds() { return s_numGlobalIds; }

protected:
    static size_t getUniqueID()
    {
        const size_t idx = s_numGlobalIds;
        s_numGlobalIds++;
        return idx;
    }

public:
    std::string m_name    = "none";
    bool   m_enabled      = true;
    bool   m_isCritical   = false;
    double m_computeTime  = 0.0;
    bool   m_enableTiming = false;

protected:
    std::function<void()> m_func = nullptr; ///< Don't allow user to call directly (must use execute)

    /// Mutex lock for thread-safe counter update
    size_t m_globalId = static_cast<size_t>(-1);
    static std::atomic<size_t> s_numGlobalIds;
};
} // namespace imstk