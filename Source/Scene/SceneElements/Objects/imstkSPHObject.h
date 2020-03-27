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
#include "imstkDynamicalModel.h"
#include "imstkSPHModel.h"

namespace imstk
{
///
/// \class SPHObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation
///
class SPHObject : public DynamicObject
{
public:
    ///
    /// \brief Constructor
    ///
    explicit SPHObject(const std::string& name);

    ///
    /// \brief Destructor
    ///
    virtual ~SPHObject() override = default;

    ///
    /// \brief Initialize the SPH scene object
    ///
    bool initialize() override;

    ///
    /// \brief Get the SPH model of the object
    ///
    const std::shared_ptr<SPHModel>& getSPHModel() const { assert(m_SPHModel); return m_SPHModel; }

protected:
    std::shared_ptr<SPHModel> m_SPHModel;
};
} // end namespace imstk
