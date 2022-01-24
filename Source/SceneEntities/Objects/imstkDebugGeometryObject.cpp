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

#include "imstkDebugGeometryObject.h"
#include "imstkLineMesh.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"

namespace imstk
{
DebugGeometryObject::DebugGeometryObject() : SceneObject("DebugGeometryObject"),
    m_arrowScale(1.0),
    m_arrowColor(Color(0.0, 1.0, 0.0)),
    m_debugLineMesh(std::make_shared<LineMesh>()),
    m_debugPointSet(std::make_shared<PointSet>()),
    m_debugSurfMesh(std::make_shared<SurfaceMesh>()),
    m_triVerticesPtr(m_debugSurfMesh->getVertexPositions()),
    m_triIndicesPtr(m_debugSurfMesh->getTriangleIndices()),
    m_triColorsPtr(std::make_shared<VecDataArray<unsigned char, 3>>()),
    m_trianglesChanged(false),
    m_lineVerticesPtr(m_debugLineMesh->getVertexPositions()),
    m_lineIndicesPtr(m_debugLineMesh->getLinesIndices()),
    m_lineColorsPtr(std::make_shared<VecDataArray<unsigned char, 3>>()),
    m_linesChanged(false),
    m_pointVerticesPtr(m_debugPointSet->getVertexPositions()),
    m_pointColorsPtr(std::make_shared<VecDataArray<unsigned char, 3>>()),
    m_ptsChanged(false)
{
    // Add color attributes to each mesh
    m_debugPointSet->setVertexScalars("colors", m_pointColorsPtr);
    m_debugLineMesh->setCellScalars("colors", m_lineColorsPtr);
    m_debugSurfMesh->setCellScalars("colors", m_triColorsPtr);

    auto lineMaterial = std::make_shared<RenderMaterial>();
    lineMaterial->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    lineMaterial->setRecomputeVertexNormals(false);
    lineMaterial->setBackFaceCulling(false);
    lineMaterial->setLineWidth(20.0);
    lineMaterial->setColor(Color::Blue);

    auto lineModel = std::make_shared<VisualModel>();
    lineModel->setGeometry(m_debugLineMesh);
    lineModel->setRenderMaterial(lineMaterial);
    addVisualModel(lineModel);

    auto pointMaterial = std::make_shared<RenderMaterial>();
    pointMaterial->setDisplayMode(RenderMaterial::DisplayMode::Points);
    pointMaterial->setRecomputeVertexNormals(false);
    pointMaterial->setBackFaceCulling(false);
    pointMaterial->setPointSize(10.0);
    pointMaterial->setColor(Color::Red);

    auto pointModel = std::make_shared<VisualModel>();
    pointModel->setGeometry(m_debugPointSet);
    pointModel->setRenderMaterial(pointMaterial);
    addVisualModel(pointModel);

    auto faceMaterial = std::make_shared<RenderMaterial>();
    faceMaterial->setRecomputeVertexNormals(false);
    faceMaterial->setBackFaceCulling(false);
    faceMaterial->setColor(Color::Orange);

    auto faceModel = std::make_shared<VisualModel>();
    faceModel->setGeometry(m_debugSurfMesh);
    faceModel->setRenderMaterial(faceMaterial);
    addVisualModel(faceModel);
}

void
DebugGeometryObject::addLine(const Vec3d& a, const Vec3d& b)
{
    addLine(a, b, getVisualModel(0)->getRenderMaterial()->getColor());
}

void
DebugGeometryObject::addLine(const Vec3d& a, const Vec3d& b, const Color& color)
{
    const int startI = static_cast<int>(m_lineVerticesPtr->size());
    m_lineVerticesPtr->push_back(a);
    m_lineVerticesPtr->push_back(b);
    m_lineIndicesPtr->push_back(Vec2i(startI, startI + 1));

    Eigen::Matrix<unsigned char, 3, 1> col(
        static_cast<unsigned char>(color.r * 255.0),
        static_cast<unsigned char>(color.g * 255.0),
        static_cast<unsigned char>(color.b * 255.0));
    m_lineColorsPtr->push_back(col);

    m_linesChanged = true;
}

void
DebugGeometryObject::addTriangle(const Vec3d& a, const Vec3d& b, const Vec3d& c)
{
    addTriangle(a, b, c, getVisualModel(2)->getRenderMaterial()->getColor());
}

void
DebugGeometryObject::addTriangle(const Vec3d& a, const Vec3d& b, const Vec3d& c, const Color& color)
{
    const int startI = static_cast<int>(m_triVerticesPtr->size());
    m_triVerticesPtr->push_back(a);
    m_triVerticesPtr->push_back(b);
    m_triVerticesPtr->push_back(c);

    m_triIndicesPtr->push_back(Vec3i(startI, startI + 1, startI + 2));

    Eigen::Matrix<unsigned char, 3, 1> col(
        static_cast<unsigned char>(color.r * 255.0),
        static_cast<unsigned char>(color.g * 255.0),
        static_cast<unsigned char>(color.b * 255.0));
    m_triColorsPtr->push_back(col);

    m_trianglesChanged = true;
}

void
DebugGeometryObject::addPoint(const Vec3d& a)
{
    addPoint(a, getVisualModel(1)->getRenderMaterial()->getColor());
}

void
DebugGeometryObject::addPoint(const Vec3d& a, const Color& color)
{
    m_pointVerticesPtr->push_back(a);

    Eigen::Matrix<unsigned char, 3, 1> col(
        static_cast<unsigned char>(color.r * 255.0),
        static_cast<unsigned char>(color.g * 255.0),
        static_cast<unsigned char>(color.b * 255.0));
    m_pointColorsPtr->push_back(col);

    m_ptsChanged = true;
}

void
DebugGeometryObject::addArrow(const Vec3d& start, const Vec3d& end)
{
    addArrow(start, end, m_arrowColor);
}

void
DebugGeometryObject::addArrow(const Vec3d& start, const Vec3d& end, const Color& color)
{
    const Vec3d scaledEnd = start + (end - start) * m_arrowScale;

    const Vec3d  diff   = scaledEnd - start;
    const double length = diff.norm();
    const Vec3d  tan    = Vec3d(1.0, 0.0, 0.0).cross(diff).normalized();

    addLine(start, scaledEnd, color);
    addLine(scaledEnd, scaledEnd - diff * 0.2 + tan * length * 0.2, color);
    addLine(scaledEnd, scaledEnd - diff * 0.2 - tan * length * 0.2, color);
}

void
DebugGeometryObject::clear()
{
    // \todo: Resize 0 shouldn't reallocate
    m_triVerticesPtr->resize(0);
    m_triIndicesPtr->resize(0);
    m_triColorsPtr->resize(0);

    m_lineIndicesPtr->resize(0);
    m_lineVerticesPtr->resize(0);
    m_lineColorsPtr->resize(0);

    m_pointVerticesPtr->resize(0);
    m_pointColorsPtr->resize(0);

    m_triVerticesPtr->postModified();
    m_triIndicesPtr->postModified();
    m_triColorsPtr->postModified();
    m_lineIndicesPtr->postModified();
    m_lineVerticesPtr->postModified();
    m_lineColorsPtr->postModified();
    m_pointVerticesPtr->postModified();
    m_pointColorsPtr->postModified();
}

void
DebugGeometryObject::visualUpdate()
{
    if (m_trianglesChanged)
    {
        m_trianglesChanged = false;
        m_triVerticesPtr->postModified();
        m_triIndicesPtr->postModified();
        m_triColorsPtr->postModified();
    }
    if (m_linesChanged)
    {
        m_linesChanged = false;
        m_lineVerticesPtr->postModified();
        m_lineIndicesPtr->postModified();
        m_lineColorsPtr->postModified();
    }
    if (m_ptsChanged)
    {
        m_ptsChanged = false;
        m_pointVerticesPtr->postModified();
        m_pointColorsPtr->postModified();
    }
}

std::shared_ptr<RenderMaterial>
DebugGeometryObject::getPointMaterial() const
{
    return m_visualModels[1]->getRenderMaterial();
}

std::shared_ptr<RenderMaterial>
DebugGeometryObject::getLineMaterial() const
{
    return m_visualModels[0]->getRenderMaterial();
}

std::shared_ptr<RenderMaterial>
DebugGeometryObject::getFaceMaterial() const
{
    return m_visualModels[2]->getRenderMaterial();
}

void
DebugGeometryObject::setLineWidth(const double width)
{
    if (m_visualModels.size() > 0)
    {
        getVisualModel(0)->getRenderMaterial()->setLineWidth(width);
    }
}

void
DebugGeometryObject::setTriColor(const Color& color)
{
    if (m_visualModels.size() > 2)
    {
        getVisualModel(2)->getRenderMaterial()->setColor(color);
    }
}

void
DebugGeometryObject::setLineColor(const Color& color)
{
    if (m_visualModels.size() > 0)
    {
        getVisualModel(0)->getRenderMaterial()->setColor(color);
    }
}

void
DebugGeometryObject::setPointColor(const Color& color)
{
    if (m_visualModels.size() > 1)
    {
        getVisualModel(1)->getRenderMaterial()->setColor(color);
    }
}

void
DebugGeometryObject::setArrowColor(const Color& color)
{
    m_arrowColor = color;
}

void
DebugGeometryObject::setPointSize(const double size)
{
    if (m_visualModels.size() > 1)
    {
        getVisualModel(1)->getRenderMaterial()->setPointSize(size);
    }
}

int
DebugGeometryObject::getNumPoints() const
{
    return m_debugPointSet->getNumVertices();
}

int
DebugGeometryObject::getNumLines() const
{
    return m_debugLineMesh->getNumLines();
}

int
DebugGeometryObject::getNumTriangles() const
{
    return m_debugSurfMesh->getNumTriangles();
}
} // namespace imstk