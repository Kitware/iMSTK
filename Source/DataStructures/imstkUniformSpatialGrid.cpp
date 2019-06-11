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
#include "imstkUniformSpatialGrid.h"

namespace imstk
{
template<class Real, class CellData>
void UniformSpatialGrid<Real, CellData>::initialize(const Vec3r& lowerCorner, const Vec3r& upperCorner, const Real cellSize)
{
    assert(cellSize > 0);
    m_LowerCorner = lowerCorner;
    m_UpperCorner = upperCorner;

    m_CellSize    = cellSize;
    m_InvCellSize = Real(1.0) / m_CellSize;

    m_NTotalCells = 1u;
    for(int i = 0; i < 3; ++i)
    {
        m_Resolution[i] = static_cast<unsigned int>(std::ceil((m_UpperCorner[i] - m_LowerCorner[i]) / m_CellSize));
        m_NTotalCells *= m_Resolution[i];
    }

    if(m_NTotalCells == 0)
    {
        LOG(FATAL) << "Invalid grid size: [" +
            std::to_string(m_LowerCorner[0]) + ", " + std::to_string(m_LowerCorner[1]) + ", " + std::to_string(m_LowerCorner[2]) + "] => " +
            std::to_string(m_UpperCorner[0]) + ", " + std::to_string(m_UpperCorner[1]) + ", " + std::to_string(m_UpperCorner[2]) + "], " +
            "cellSize = " + std::to_string(m_CellSize);
    }

    // cell data must be resized to equal to number of cells
    m_CellData.resize(m_NTotalCells);
}
} // end namespace imstk

// Explicit instantiate class UniformSpatialGrid
template class imstk::UniformSpatialGrid<double, std::vector<size_t>>;