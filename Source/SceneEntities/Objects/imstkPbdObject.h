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
class PbdModel;

///
/// \class PbdObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation, implements the PbdModel and PbdSolver
///
class PbdObject : public DynamicObject
{
public:
    PbdObject(const std::string& name) : DynamicObject(name) { }
    virtual ~PbdObject() override = default;

public:
    virtual const std::string getTypeName() const override { return "PbdObject"; }

    ///
    /// \biref Get the Pbd model of the object
    ///
    std::shared_ptr<PbdModel> getPbdModel();

    ///
    /// \brief Initialize the pbd scene object
    ///
    bool initialize() override;

protected:
    std::shared_ptr<PbdModel> m_pbdModel = nullptr;  ///> PBD mathematical model
};
} // imstk
