/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDebugGeometryModel.h"
#include "imstkLineMesh.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"
#include "imstkEntity.h"

namespace imstk
{
DebugGeometryModel::DebugGeometryModel(const std::string& name) : SceneBehaviour(name),
    m_arrowScale(1.0),
    m_arrowColor(Color(0.0, 1.0, 0.0)),
    m_debugLineMesh(std::make_shared<LineMesh>()),
    m_debugPointSet(std::make_shared<PointSet>()),
    m_debugSurfMesh(std::make_shared<SurfaceMesh>()),
    m_triVerticesPtr(m_debugSurfMesh->getVertexPositions()),
    m_triIndicesPtr(m_debugSurfMesh->getCells()),
    m_triColorsPtr(std::make_shared<VecDataArray<unsigned char, 3>>()),
    m_trianglesChanged(false),
    m_lineVerticesPtr(m_debugLineMesh->getVertexPositions()),
    m_lineIndicesPtr(m_debugLineMesh->getCells()),
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

    m_debugLineModel = std::make_shared<VisualModel>();
    m_debugLineModel->setGeometry(m_debugLineMesh);
    m_debugLineModel->setRenderMaterial(lineMaterial);

    auto pointMaterial = std::make_shared<RenderMaterial>();
    pointMaterial->setDisplayMode(RenderMaterial::DisplayMode::Points);
    pointMaterial->setRecomputeVertexNormals(false);
    pointMaterial->setBackFaceCulling(false);
    pointMaterial->setPointSize(10.0);
    pointMaterial->setColor(Color::Red);

    m_debugPointModel = std::make_shared<VisualModel>();
    m_debugPointModel->setGeometry(m_debugPointSet);
    m_debugPointModel->setRenderMaterial(pointMaterial);

    auto faceMaterial = std::make_shared<RenderMaterial>();
    faceMaterial->setRecomputeVertexNormals(false);
    faceMaterial->setBackFaceCulling(false);
    faceMaterial->setColor(Color::Orange);

    m_debugSurfModel = std::make_shared<VisualModel>();
    m_debugSurfModel->setGeometry(m_debugSurfMesh);
    m_debugSurfModel->setRenderMaterial(faceMaterial);
}

void
DebugGeometryModel::init()
{
    std::shared_ptr<Entity> entity = m_entity.lock();
    CHECK(entity != nullptr) << "DebugGeometryModel must have entity to initialize";
    if (!entity->containsComponent(m_debugPointModel))
    {
        m_debugPointModel->setName(entity->getName() + "_DebugPointModel");
        entity->addComponent(m_debugPointModel);
    }
    if (!entity->containsComponent(m_debugLineModel))
    {
        m_debugLineModel->setName(entity->getName() + "_DebugLineModel");
        entity->addComponent(m_debugLineModel);
    }
    if (!entity->containsComponent(m_debugSurfModel))
    {
        m_debugSurfModel->setName(entity->getName() + "_DebugSurfModel");
        entity->addComponent(m_debugSurfModel);
    }
}

void
DebugGeometryModel::addLine(const Vec3d& a, const Vec3d& b)
{
    addLine(a, b, m_debugLineModel->getRenderMaterial()->getColor());
}

void
DebugGeometryModel::addLine(const Vec3d& a, const Vec3d& b, const Color& color)
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
DebugGeometryModel::addTriangle(const Vec3d& a, const Vec3d& b, const Vec3d& c)
{
    addTriangle(a, b, c, m_debugSurfModel->getRenderMaterial()->getColor());
}

void
DebugGeometryModel::addTriangle(const Vec3d& a, const Vec3d& b, const Vec3d& c, const Color& color)
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
DebugGeometryModel::addPoint(const Vec3d& a)
{
    addPoint(a, m_debugPointModel->getRenderMaterial()->getColor());
}

void
DebugGeometryModel::addPoint(const Vec3d& a, const Color& color)
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
DebugGeometryModel::addArrow(const Vec3d& start, const Vec3d& end)
{
    addArrow(start, end, m_arrowColor);
}

void
DebugGeometryModel::addArrow(const Vec3d& start, const Vec3d& end, const Color& color)
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
DebugGeometryModel::clear()
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
DebugGeometryModel::visualUpdate(const double&)
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
DebugGeometryModel::getPointMaterial() const
{
    return m_debugPointModel->getRenderMaterial();
}

std::shared_ptr<RenderMaterial>
DebugGeometryModel::getLineMaterial() const
{
    return m_debugLineModel->getRenderMaterial();
}

std::shared_ptr<RenderMaterial>
DebugGeometryModel::getFaceMaterial() const
{
    return m_debugSurfModel->getRenderMaterial();
}

void
DebugGeometryModel::setLineWidth(const double width)
{
    m_debugLineModel->getRenderMaterial()->setLineWidth(width);
}

void
DebugGeometryModel::setTriColor(const Color& color)
{
    m_debugSurfModel->getRenderMaterial()->setColor(color);
}

void
DebugGeometryModel::setLineColor(const Color& color)
{
    m_debugLineModel->getRenderMaterial()->setColor(color);
}

void
DebugGeometryModel::setPointColor(const Color& color)
{
    m_debugPointModel->getRenderMaterial()->setColor(color);
}

void
DebugGeometryModel::setArrowColor(const Color& color)
{
    m_arrowColor = color;
}

void
DebugGeometryModel::setPointSize(const double size)
{
    m_debugPointModel->getRenderMaterial()->setPointSize(size);
}

int
DebugGeometryModel::getNumPoints() const
{
    return m_debugPointSet->getNumVertices();
}

int
DebugGeometryModel::getNumLines() const
{
    return m_debugLineMesh->getNumCells();
}

int
DebugGeometryModel::getNumTriangles() const
{
    return m_debugSurfMesh->getNumCells();
}
} // namespace imstk