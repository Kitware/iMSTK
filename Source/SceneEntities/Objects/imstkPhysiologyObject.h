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
  class PhysiologyModel;

  ///
  /// \class PhysiologyObject
  ///
  /// \brief Base class for Physiology objects
  ///
  class PhysiologyObject : public DynamicObject
  {
  public:
    explicit PhysiologyObject(const std::string& name);

    virtual ~PhysiologyObject() override = default;

  public:
    ///
    /// \brief Initialize the Physiology scene object
    ///
    bool initialize() override;

    std::shared_ptr<PhysiologyModel> getPhysiologyModel();

  protected:
    std::shared_ptr<PhysiologyModel> m_PhysiologyModel = nullptr;
  };
} // end namespace imstk
