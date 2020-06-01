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

#include <memory>
#include <string>

// imstk
#include "imstkDynamicObject.h"
#include "imstkMath.h"

namespace imstk
{
class FEMDeformableBodyModel;

///
/// \class DeformableObject
///
/// \brief Scene objects that can deform
///
class FeDeformableObject : public DynamicObject
{
public:
    explicit FeDeformableObject(const std::string& name) : DynamicObject(name) { m_type = Type::FEMDeformable; }
    FeDeformableObject() = delete;

    ~FeDeformableObject() = default;

public:
    ///
    /// \brief Initialize the deformable object
    ///
    bool initialize() override;

    std::shared_ptr<FEMDeformableBodyModel> getFEMModel() const { return m_femModel; }

protected:
    std::shared_ptr<FEMDeformableBodyModel> m_femModel = nullptr;
};
} // imstk
