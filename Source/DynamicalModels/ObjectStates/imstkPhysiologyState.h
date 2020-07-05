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

#include "imstkMath.h"

namespace imstk
{
///
/// \class PhysiologyState
/// \brief State of Physiology model
///
class PhysiologyState // keep class for now in case we need it in future
{
public:
    ///
    /// \brief Default constructor/destructor
    ///
    PhysiologyState() = default;
    virtual ~PhysiologyState() = default;

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const std::shared_ptr<PhysiologyState>&) {};

private:
};
} // end namespace imstk
