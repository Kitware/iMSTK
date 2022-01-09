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

#include "imstkColor.h"
#include "imstkMath.h"
#include "imstkSceneObject.h"

namespace imstk
{
class LineMesh;
class PointSet;
class RenderMaterial;
class SurfaceMesh;
template<typename T, int N> class VecDataArray;

///
/// \class DebugGeometryObject
///
/// \brief Class for quickly rendering and showing various primivites such as
/// line segments, triangles, arrows, points
/// \todo: Allow per vertex and per cell coloring
///
class DebugGeometryObject : public SceneObject
{
public:
    DebugGeometryObject();
    virtual ~DebugGeometryObject() = default;

    ///
    /// \brief Adds a line to the debug lines with default color
    ///
    void addLine(const Vec3d& a, const Vec3d& b);

    ///
    /// \brief Adds a line to the debug lines with specified color
    ///
    void addLine(const Vec3d& a, const Vec3d& b, const Color& color);

    ///
    /// \brief Adds a triangle to the debug triangles with default color
    ///
    void addTriangle(const Vec3d& a, const Vec3d& b, const Vec3d& c);

    ///
    /// \brief Adds a triangle to the debug triangles with specified color
    ///
    void addTriangle(const Vec3d& a, const Vec3d& b, const Vec3d& c, const Color& color);

    ///
    /// \brief Adds a point to the debug points
    /// \param point to add
    ///
    void addPoint(const Vec3d& a);

    ///
    /// \brief Adds a point to the debug points
    /// \param point to add
    /// \param color of the point
    ///
    void addPoint(const Vec3d& a, const Color& color);

    ///
    /// \brief Adds an arrow to the debug arrows
    /// \param tail of arrow
    /// \param tip of arrow
    ///
    void addArrow(const Vec3d& start, const Vec3d& end);

    ///
    /// \brief Adds an arrow to the debug arrows
    /// \param tail of arrow
    /// \param tip of arrow
    /// \param color of the arrow
    ///
    void addArrow(const Vec3d& start, const Vec3d& end, const Color& color);

    ///
    /// \brief Clears all primitives
    ///
    void clear();

/// Accessors
    std::shared_ptr<RenderMaterial> getPointMaterial() const;
    std::shared_ptr<RenderMaterial> getLineMaterial() const;
    std::shared_ptr<RenderMaterial> getFaceMaterial() const;

    ///
    /// \brief Update the primitives
    ///
    void visualUpdate() override;

    void setLineWidth(const double width);

    void setTriColor(const Color& color);
    void setLineColor(const Color& color);
    void setPointColor(const Color& color);
    void setArrowColor(const Color& color);

    void setPointSize(const double size);

    void setArrowScale(const double arrowScale) { m_arrowScale = arrowScale; }

    int getNumPoints() const;
    int getNumLines() const;
    int getNumTriangles() const;

protected:
    double m_arrowScale;
    Color  m_arrowColor;

    std::shared_ptr<LineMesh>    m_debugLineMesh;
    std::shared_ptr<PointSet>    m_debugPointSet;
    std::shared_ptr<SurfaceMesh> m_debugSurfMesh;

    std::shared_ptr<VecDataArray<double, 3>> m_triVerticesPtr;
    std::shared_ptr<VecDataArray<int, 3>>    m_triIndicesPtr;
    std::shared_ptr<VecDataArray<unsigned char, 3>> m_triColorsPtr;
    bool m_trianglesChanged;

    std::shared_ptr<VecDataArray<double, 3>> m_lineVerticesPtr;
    std::shared_ptr<VecDataArray<int, 2>>    m_lineIndicesPtr;
    std::shared_ptr<VecDataArray<unsigned char, 3>> m_lineColorsPtr;
    bool m_linesChanged;

    std::shared_ptr<VecDataArray<double, 3>> m_pointVerticesPtr;
    std::shared_ptr<VecDataArray<unsigned char, 3>> m_pointColorsPtr;
    bool m_ptsChanged;
};
}