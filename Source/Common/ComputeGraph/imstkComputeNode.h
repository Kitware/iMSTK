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
/// \class ComputeNode
///
/// \brief Base class for compute graph nodes
///
class ComputeNode
{
public:
    ComputeNode() = default;
    ComputeNode(std::function<void()> func, std::string name = "none", bool critical = false) :
        m_func(func), m_name(name), m_critical(critical), m_elapsedTime(0.0)
    {
    }

    virtual ~ComputeNode() = default;

public:
    void setFunction(std::function<void()> func) { this->m_func = func; }
    void setEnabled(bool enabled) { this->m_enabled = enabled; }

    ///
    /// \brief Returns true if function is nullptr
    ///
    bool isFunctional() const { return m_func != nullptr; }

    // Executes
    virtual void execute();

public:
    std::string m_name = "none";
    bool m_enabled     = true;
    bool m_critical    = false;
    double m_elapsedTime = 0.0;
    bool m_enableBenchmarking = false;

protected:
    std::function<void()> m_func = nullptr; ///> Don't allow user to call directly (must use execute)
};
}