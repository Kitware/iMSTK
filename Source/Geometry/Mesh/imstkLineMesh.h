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

#include "imstkCellMesh.h"

namespace imstk
{
///
/// \class LineMesh
///
/// \brief Base class for all volume mesh types
///
class LineMesh : public CellMesh<2>
{
public:
    LineMesh() = default;
    ~LineMesh() override = default;

    int getNumLines() const { return getNumCells(); }
    void setLinesIndices(std::shared_ptr<VecDataArray<int, 2>> indices) { setCells(indices); }
    std::shared_ptr<VecDataArray<int, 2>> getLinesIndices() const { return getCells(); }

    IMSTK_TYPE_NAME(LineMesh)
};
} // namespace imstk
