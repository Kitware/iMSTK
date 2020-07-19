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

#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkLogger.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkTextureManager.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkTrivialProducer.h>
#include <vtkVersion.h>

namespace imstk
{
VTKSurfaceMeshRenderDelegate::VTKSurfaceMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New()),
    m_mappedNormalArray(vtkSmartPointer<vtkDoubleArray>::New()),
    m_mappedTangentArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_visualModel = visualModel;

    auto geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());

    // Map vertices
    StdVectorOfVec3d& vertices = geometry->getVertexPositionsNotConst();
    double*           vertData = reinterpret_cast<double*>(vertices.data());
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(vertData, vertices.size() * 3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Copy cells
    auto      cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType cell[3];
    for (const auto& t : geometry->getTrianglesVertices())
    {
        for (size_t i = 0; i < 3; ++i)
        {
            cell[i] = t[i];
        }
        cells->InsertNextCell(3, cell);
    }

    // Create PolyData
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetPolys(cells);

    // Map normals
    geometry->computeVertexNormals();

    StdVectorOfVec3d& normals    = geometry->getVertexNormalsNotConst();
    double*           normalData = reinterpret_cast<double*>(normals.data());
    m_mappedNormalArray->SetNumberOfComponents(3);
    m_mappedNormalArray->SetArray(normalData, normals.size() * 3, 1);
    polydata->GetPointData()->SetNormals(m_mappedNormalArray);

    // Create connection source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(polydata);
    geometry->m_dataModified = false;

    // Setup texture coordinates
    if (geometry->getDefaultTCoords() != "")
    {
        // Convert texture coordinates
        auto tcoords = geometry->getPointDataArray(geometry->getDefaultTCoords());
        if (tcoords == nullptr)
        {
            LOG(WARNING) << "No default texture coordinates array for geometry " << geometry;
        }
        else
        {
            auto vtkTCoords = vtkSmartPointer<vtkFloatArray>::New();
            vtkTCoords->SetNumberOfComponents(2);
            vtkTCoords->SetName(geometry->getDefaultTCoords().c_str());

            for (auto const tcoord : *tcoords)
            {
                float tuple[2] = { tcoord[0], tcoord[1] };
                vtkTCoords->InsertNextTuple(tuple);
            }

            polydata->GetPointData()->SetTCoords(vtkTCoords);
        }
    }

    // Update tangents
    if (geometry->getVertexTangents().size() > 0)
    {
        auto tangents = vtkSmartPointer<vtkFloatArray>::New();
        tangents->SetName("tangents");
        tangents->SetNumberOfComponents(3);

        for (auto const tangent : geometry->getVertexTangents())
        {
            float tempTangent[3] = { (float)tangent[0],
                                     (float)tangent[1],
                                     (float)tangent[2] };
            tangents->InsertNextTuple(tempTangent);
        }
        polydata->GetPointData()->SetTangents(tangents);
    }

    // Update Transform, Render Properties
    this->update();
    this->setUpMapper(source->GetOutputPort(), m_visualModel);

    m_isMesh = true;

    //m_mapper->setIsSurfaceMapper(true);
}

void
VTKSurfaceMeshRenderDelegate::updateDataSource()
{
    auto geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());

    if (geometry->m_dataModified)
    {
        geometry->computeVertexNormals();

        StdVectorOfVec3d& normals    = geometry->getVertexNormalsNotConst();
        double*           normalData = reinterpret_cast<double*>(normals.data());
        m_mappedNormalArray->SetNumberOfComponents(3);
        m_mappedNormalArray->SetArray(normalData, normals.size() * 3, 1);
        this->m_mapper->GetInput()->GetPointData()->SetNormals(m_mappedNormalArray);

        m_mappedVertexArray->Modified();
        m_mappedNormalArray->Modified();
        geometry->m_dataModified = false;
    }
}

void
VTKSurfaceMeshRenderDelegate::initializeTextures(TextureManager<VTKTextureDelegate>& textureManager)
{
    auto material = m_visualModel->getRenderMaterial();
    if (material == nullptr)
    {
        return;
    }

    unsigned int currentUnit = 0;

    // Go through all of the textures
    for (int unit = 0; unit < (int)Texture::Type::None; unit++)
    {
        // Get imstk texture
        auto texture = material->getTexture((Texture::Type)unit);
        if (std::strcmp(texture->getPath().c_str(), "") == 0)
        {
            continue;
        }

        // Get vtk texture
        auto textureDelegate = textureManager.getTextureDelegate(texture);

        /* /!\ VTKTextureWrapMode not yet supported in VTK 7
        * See here for some work that needs to be imported back to upstream:
        * https://gitlab.kitware.com/iMSTK/vtk/commit/62a7ecd8a5f54e243c26960de22d5d1d23ef932b
        *
        texture->SetWrapMode(vtkTexture::VTKTextureWrapMode::ClampToBorder);

        * /!\ MultiTextureAttribute not yet supported in VTK 7
        * See here for some work that needs to be imported back to upstream:
        * https://gitlab.kitware.com/iMSTK/vtk/commit/ae373026755db42b6fdce5093109ef1a39a76340
        *
        // Link texture unit to texture attribute
        m_mapper->MapDataArrayToMultiTextureAttribute(unit, tCoordsName.c_str(),
                                                    vtkDataObject::FIELD_ASSOCIATION_POINTS);
        */

        // Set texture
        auto currentTexture = textureDelegate->getTexture();

#if (VTK_MAJOR_VERSION <= 8 && VTK_MINOR_VERSION <= 1)
        m_actor->GetProperty()->SetTexture(currentUnit, currentTexture);
#else
        if (material->getShadingModel() == RenderMaterial::ShadingModel::PBR)
        {
            switch (texture->getType())
            {
            case Texture::Type::Diffuse:
            {
                m_actor->GetProperty()->SetBaseColorTexture(currentTexture);
                break;
            }
            case Texture::Type::Normal:
            {
                m_actor->GetProperty()->SetNormalTexture(currentTexture);
                m_actor->GetProperty()->SetNormalScale(material->getNormalStrength());
                break;
            }
            case Texture::Type::AmbientOcclusion:
            {
                m_actor->GetProperty()->SetORMTexture(currentTexture);
                m_actor->GetProperty()->SetOcclusionStrength(material->getOcclusionStrength());
                break;
            }
            default:
            {
            }
            }
        }
        else
        {
            m_actor->GetProperty()->SetTexture(textureDelegate->getTextureName().c_str(), currentTexture);
        }

#endif

        currentUnit++;
    }
}
} // imstk
