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

    actorProperty->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    actorProperty->SetDiffuse(material->getDiffuse());
    actorProperty->SetAmbientColor(ambientColor.r, ambientColor.g, ambientColor.b);
    actorProperty->SetAmbient(material->getAmbient());
    actorProperty->SetSpecularColor(specularColor.r, specularColor.g, specularColor.b);
    actorProperty->SetSpecularPower(material->getSpecularPower());
    actorProperty->SetSpecular(material->getSpecular());
    actorProperty->SetMetallic(material->getMetalness());
    actorProperty->SetRoughness(material->getRoughness());

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
        /*actorProperty->UseImageBasedLightingOn();
            actorProperty->SetEnvironmentCubeMap(getVTKTexture(cubemap));*/

        actorProperty->SetInterpolationToPBR();

        // configure the basic properties
        //actorProperty->SetColor(surfaceColor.r, surfaceColor.g, surfaceColor.b);
    }
    else if (material->getShadingModel() == RenderMaterial::ShadingModel::Phong)
    {
        actorProperty->SetInterpolationToPhong();
    }
    else if (material->getShadingModel() == RenderMaterial::ShadingModel::Gouraud)
    {
        actorProperty->SetInterpolationToGouraud();
    }
    else
    {
        actorProperty->SetInterpolationToFlat();
    }

    // Display mode
    switch (material->getDisplayMode())
    {
    case RenderMaterial::DisplayMode::Wireframe:
        actorProperty->SetRepresentationToWireframe();
        actorProperty->SetEdgeVisibility(false);
        // actorProperty->SetRenderLinesAsTubes(true);
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

    m_actor->SetVisibility(m_visualModel->isVisible());
}
}