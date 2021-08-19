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

#include "imstkPointSet.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class VolumetricMesh
///
/// \brief Base class for all volume mesh types
///
class VolumetricMesh : public PointSet
{
public:
    ///
    /// \brief Destructor
    ///
    virtual ~VolumetricMesh() override = default;

public:
    ///
    /// \brief Compute the conforming surface mesh
    ///
    virtual std::shared_ptr<SurfaceMesh> extractSurfaceMesh() { return nullptr; }

protected:
    VolumetricMesh(const std::string& name = std::string("")) : PointSet(name) {}
};
} // imstk
