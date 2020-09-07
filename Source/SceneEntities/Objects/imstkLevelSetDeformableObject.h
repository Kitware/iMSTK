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

#include "imstkDynamicObject.h"

namespace imstk
{
class LevelSetModel;

///
/// \class PbdObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation, implements the PbdModel and PbdSolver
///
class LevelSetDeformableObject : public DynamicObject
{
public:
    explicit LevelSetDeformableObject(const std::string& name) : DynamicObject(name)
    {
        m_type = SceneObject::Type::LevelSetDeformable;
    }

    LevelSetDeformableObject() = delete;

    virtual ~LevelSetDeformableObject() override = default;

public:
    ///
    /// \biref Get the Pbd model of the object
    ///
    std::shared_ptr<LevelSetModel> getLevelSetModel();

    ///
    /// \brief Initialize the pbd scene object
    ///
    bool initialize() override;

protected:
    std::shared_ptr<LevelSetModel> m_levelSetModel = nullptr;  ///> PBD mathematical model
};
} // imstk
