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

#include "imstkGeometry.h"

namespace imstk
{
///
/// \class ImplicitGeometry
///
/// \brief Abstract base class for any implicit geometrical representation
///
class ImplicitGeometry : public Geometry
{
public:
    virtual ~ImplicitGeometry() override = default;

protected:
    ImplicitGeometry(Type geomType, std::string name = "") : Geometry(geomType, name) { }

public:
    ///
    /// \brief Returns signed distance to surface at pos
    ///
    virtual double getFunctionValue(const Vec3d& pos) const = 0;

    ///
    /// \brief Returns gradient of signed distance at pos
    ///
    virtual Vec3d getFunctionGrad(const Vec3d& pos) const = 0;
};
}
