/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVisualModel.h"
#include "imstkRenderMaterial.h"
#include "imstkGeometry.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkImageData.h"
#include "imstkLogger.h"

namespace imstk
{
VisualModel::VisualModel(const std::string& name) : Component(name),
    m_delegateHint(""),
    m_geometry(nullptr),
    m_renderMaterial(std::make_shared<RenderMaterial>()),
    m_isVisible(true)
{
}

const std::string
VisualModel::getDelegateHint() const
{
    // Prioritize user set delegate hint
    if (m_delegateHint != "")
    {
        return m_delegateHint;
    }

    // Early Exit
    if (m_geometry == nullptr)
    {
        return "";
    }

    // Special Handling various rendermaterials
    if (getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Fluid)
    {
        if (std::dynamic_pointer_cast<PointSet>(m_geometry) != nullptr)
        {
            return "Fluid";
        }
        else
        {
            LOG(WARNING) << "Requested DisplayMode::Fluid but <" << m_geometry->getTypeName()
                         << "> cannot be converted to PointSet using default render delegate.";
        }
    }

    if (getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::SurfaceNormals)
    {
        if (std::dynamic_pointer_cast<SurfaceMesh>(m_geometry) != nullptr)
        {
            return "SurfaceNormals";
        }
        else
        {
            LOG(WARNING) << "Requested DisplayMode::SurfaceNormals but <" << m_geometry->getTypeName()
                         << "> cannot be converted to SurfaceMesh using default render delegate.";
        }
    }

    if (getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Points)
    {
        if (std::dynamic_pointer_cast<PointSet>(m_geometry) != nullptr)
        {
            return "PointSet"; // Match Point set Type name
        }
        else
        {
            LOG(WARNING) << "Requested DisplayMode::Points but <" << m_geometry->getTypeName()
                         << "> cannot be converted to PointSet using default render delegate.";
        }
    }

    return m_geometry->getTypeName();
}

void
VisualModel::setRenderMaterial(std::shared_ptr<RenderMaterial> renderMaterial)
{
    m_renderMaterial = renderMaterial;
    this->postModified();
}

void
VisualModel::setIsVisible(const bool visible)
{
    m_isVisible = visible;
    this->postModified();
}

bool
VisualModel::getRenderDelegateCreated(Renderer* ren)
{
    if (m_renderDelegateCreated.count(ren) == 0)
    {
        m_renderDelegateCreated[ren] = false;
        return false;
    }
    else
    {
        return m_renderDelegateCreated[ren];
    }
}
} // namespace imstk
