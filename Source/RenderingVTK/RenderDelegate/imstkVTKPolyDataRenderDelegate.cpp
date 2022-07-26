/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKPolyDataRenderDelegate.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"
#include "imstkColorFunction.h"

#include <vtkActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

namespace imstk
{
void
VTKPolyDataRenderDelegate::updateRenderProperties()
{
    std::shared_ptr<RenderMaterial>    material      = m_visualModel->getRenderMaterial();
    vtkSmartPointer<vtkProperty>       actorProperty = vtkActor::SafeDownCast(m_actor)->GetProperty();
    vtkSmartPointer<vtkPolyDataMapper> polyMapper    = vtkPolyDataMapper::SafeDownCast(m_mapper);

    if (material->getScalarVisibility() && polyMapper != nullptr)
    {
        // Convert color table
        std::shared_ptr<ColorFunction>   imstkLookupTable = material->getColorLookupTable();
        const double*                    range   = imstkLookupTable->getRange().data();
        double                           spacing = (range[1] - range[0]) / imstkLookupTable->getNumberOfColors();
        vtkNew<vtkColorTransferFunction> lookupTable;
        lookupTable->SetColorSpaceToRGB();
        for (int i = 0; i < imstkLookupTable->getNumberOfColors(); i++)
        {
            const double t     = static_cast<double>(i) / imstkLookupTable->getNumberOfColors();
            const Color& color = imstkLookupTable->getColor(i);
            lookupTable->AddRGBPoint(t * (range[1] - range[0]) + range[0] + spacing * 0.5, color.r, color.g, color.b);
        }

        switch (imstkLookupTable->getColorSpace())
        {
        case ColorFunction::ColorSpace::RGB:
            lookupTable->SetColorSpaceToRGB();
            break;
        case ColorFunction::ColorSpace::HSV:
            lookupTable->SetColorSpaceToHSV();
            break;
        case ColorFunction::ColorSpace::LAB:
            lookupTable->SetColorSpaceToLab();
            break;
        case ColorFunction::ColorSpace::DIVERING:
            lookupTable->SetColorSpaceToDiverging();
            break;
        default:
            lookupTable->SetColorSpaceToRGB();
            break;
        }

        polyMapper->SetLookupTable(lookupTable);
        polyMapper->SetScalarVisibility(material->getScalarVisibility());
    }

    // Colors & Light
    const Color& diffuseColor  = material->getDiffuseColor();
    const Color& ambientColor  = material->getAmbientColor();
    const Color& specularColor = material->getSpecularColor();
    const Color& edgeColor     = material->getEdgeColor();
    const Color& vertexColor   = material->getVertexColor();
    const Color& surfaceColor  = material->getColor();
    const Color& coatColor     = material->getCoatColor();
    const Color& edgeTintColor = material->getEdgeTint();

    // Phong
    actorProperty->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    actorProperty->SetDiffuse(material->getDiffuse());
    actorProperty->SetAmbientColor(ambientColor.r, ambientColor.g, ambientColor.b);
    actorProperty->SetAmbient(material->getAmbient());
    actorProperty->SetSpecularColor(specularColor.r, specularColor.g, specularColor.b);
    actorProperty->SetSpecularPower(material->getSpecularPower());
    actorProperty->SetSpecular(material->getSpecular());

    // PBR, ORM (occlusion, roughness, metallic)
    actorProperty->SetOcclusionStrength(material->getOcclusionStrength());
    actorProperty->SetRoughness(material->getRoughness());
    actorProperty->SetMetallic(material->getMetalness());
    actorProperty->SetNormalScale(material->getNormalStrength());
    // PBR clearcoat
    actorProperty->SetAnisotropy(material->getAnisotropy());
    actorProperty->SetAnisotropyRotation(material->getAnisotropyRotation());
    actorProperty->SetBaseIOR(material->getBaseIOR());
    actorProperty->SetCoatColor(coatColor.r, coatColor.g, coatColor.b);
    actorProperty->SetCoatIOR(material->getCoatIOR());
    actorProperty->SetCoatNormalScale(material->getCoatNormalScale());
    actorProperty->SetCoatRoughness(material->getCoatRoughness());
    actorProperty->SetCoatStrength(material->getCoatStrength());
    actorProperty->SetEdgeTint(edgeTintColor.r, edgeTintColor.g, edgeTintColor.b);

    // Base
    actorProperty->SetColor(surfaceColor.r, surfaceColor.g, surfaceColor.b);
    actorProperty->SetVertexColor(vertexColor.r, vertexColor.g, vertexColor.b);
    actorProperty->SetEdgeColor(edgeColor.r, edgeColor.g, edgeColor.b);
    actorProperty->SetLineWidth(material->getLineWidth());
    actorProperty->SetPointSize(material->getPointSize());
    actorProperty->SetBackfaceCulling(material->getBackFaceCulling());
    actorProperty->SetOpacity(material->getOpacity());
    actorProperty->SetRenderPointsAsSpheres(material->getRenderPointsAsSpheres());

    if (material->getShadingModel() == RenderMaterial::ShadingModel::PBR)
    {
        actorProperty->LightingOn();
        actorProperty->SetInterpolationToPBR();
    }
    else if (material->getShadingModel() == RenderMaterial::ShadingModel::Phong)
    {
        actorProperty->LightingOn();
        actorProperty->SetInterpolationToPhong();
    }
    else if (material->getShadingModel() == RenderMaterial::ShadingModel::Gouraud)
    {
        actorProperty->LightingOn();
        actorProperty->SetInterpolationToGouraud();
    }
    else if (material->getShadingModel() == RenderMaterial::ShadingModel::Flat)
    {
        actorProperty->LightingOn();
        actorProperty->SetInterpolationToFlat();
    }
    else if (material->getShadingModel() == RenderMaterial::ShadingModel::None)
    {
        actorProperty->LightingOff();
    }

    // Display mode
    switch (material->getDisplayMode())
    {
    case RenderMaterial::DisplayMode::Wireframe:
        actorProperty->SetRepresentationToWireframe();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::Points:
        actorProperty->SetRepresentationToPoints();
        actorProperty->SetEdgeVisibility(false);
        break;
    case RenderMaterial::DisplayMode::WireframeSurface:
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(true);
        break;
    case RenderMaterial::DisplayMode::Surface:
    default:
        actorProperty->SetRepresentationToSurface();
        actorProperty->SetEdgeVisibility(false);
        actorProperty->SetVertexVisibility(false);
        break;
    }

    m_actor->SetVisibility(m_visualModel->isVisible() ? 1 : 0);
}
} // namespace imstk