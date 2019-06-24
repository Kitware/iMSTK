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
#include <array>

#include "imstkPointSet.h"

namespace imstk
{
///
/// \class LineMesh
///
/// \brief Base class for all volume mesh types
///
class LineMesh : public PointSet
{
public:

    using LineArray = std::array<size_t, 2>;

    ///
    /// \brief Constructor
    ///
    LineMesh() : PointSet(Geometry::Type::LineMesh) {}

    ///
    /// \brief Default destructor
    ///
    ~LineMesh() = default;

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  line connectivity
    ///
    void initialize(const StdVectorOfVec3d& vertices,
                    const std::vector<LineArray>& lines);

    ///
    /// \brief
    ///
    void clear() override;

    ///
    /// \brief
    ///
    void print() const override;

    ///
    /// \brief
    ///
    double getVolume() const override;

    ///
    /// \brief
    ///
    void setVertexColors(const std::vector<Color>& colors);

    ///
    /// \brief
    ///
    void setLinesVertices(const std::vector<LineArray>& lines);

    ///
    /// \brief
    ///
    size_t getNumLines();

    ///
    /// \brief
    ///
    std::vector<LineArray> getLinesVertices() const;

    ///
    /// \brief
    ///
    std::vector<Color> getVertexColors() const;

private:

    friend class VTKLineMeshRenderDelegate;

    size_t m_originalNumLines = 0;
    size_t m_maxNumLines = 0;

    std::vector<LineArray> m_lines;  ///> line connectivity
    std::vector<Color> m_vertexColors;
};
} // imstk

