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
    TaskNode() = default;
    TaskNode(std::function<void()> func, std::string name = "none", bool isCritical = false) :
        m_name(name), m_isCritical(isCritical), m_computeTime(0.0), m_func(func)
    {
    }

    virtual ~TaskNode() = default;

public:
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

public:
    std::string m_name    = "none";
    bool   m_enabled      = true;
    bool   m_isCritical   = false;
    double m_computeTime  = 0.0;
    bool   m_enableTiming = false;

protected:
    std::function<void()> m_func = nullptr; ///> Don't allow user to call directly (must use execute)
};
}