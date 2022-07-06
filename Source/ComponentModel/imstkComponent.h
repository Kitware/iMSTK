///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#pragma once

#include <string>

namespace imstk
{
///
/// \class Component
///
/// \brief Represents a part of a component, involved in a system
///
class Component
{
protected:
    Component(const std::string& name = "Component") : m_name(name) { }

public:
    virtual ~Component() = default;

protected:
    std::string m_name;
};

///
/// \class Behaviour
///
/// \brief A Behaviour represents a single component system
///
template<typename UpdateInfo>
class Behaviour : public Component
{
protected:
    Behaviour(const std::string& name = "Behaviour") : Component(name) { }

public:
    ~Behaviour() override = default;

    virtual void update(const UpdateInfo& updateData) = 0;
};
} // namespace imstk