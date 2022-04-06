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
#include "imstkMacros.h"

namespace imstk
{
class SphModel;

///
/// \class SphObject
///
/// \brief Base class for scene objects that move and/or deform under
/// smooth particle hydrodynamics
///
class SphObject : public DynamicObject
{
public:
    SphObject(const std::string& name) : DynamicObject(name) { }
    ~SphObject() override = default;

    IMSTK_TYPE_NAME(SphObject)

    ///
    /// \brief Get the model governing the Sph fluid dynamics of this object
    ///
    std::shared_ptr<SphModel> getSphModel();

    ///
    /// \brief Initialize the SphObject
    ///
    bool initialize() override;

protected:
    std::shared_ptr<SphModel> m_sphModel = nullptr;
};
} // namespace imstk