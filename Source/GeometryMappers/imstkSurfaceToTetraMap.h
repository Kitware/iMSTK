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

#include "imstkOneToOneMap.h"
#include "imstkMath.h"
#include "imstkTypes.h"

namespace imstk
{
///
/// \class SurfaceToTetraMap
///
/// \brief SurfaceToTetrahedralMap serves as a OneToOneMap but also
/// maps tets to triangle faces.
///
class SurfaceToTetraMap : public OneToOneMap
{
public:
    SurfaceToTetraMap();
    SurfaceToTetraMap(
        std::shared_ptr<Geometry> parent,
        std::shared_ptr<Geometry> child);
    ~SurfaceToTetraMap() override = default;

    ///
    /// \brief Compute the map
    ///
    void compute() override;

    ///
    /// \brief Compute tet vertex id to surf vertex id map
    /// 
    void computeTriToTetMap(std::unordered_map<int, int>& triToTetMap);

    ///
    /// \brief Get the tet id that contains the triangle
    /// 
    int getParentTetId(const int triId) const;

public:
    std::unordered_map<int, int> m_triToTetMap;
};
} // namespace imstk