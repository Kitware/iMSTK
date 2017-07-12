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

#include "imstkSurfaceMesh.h"

#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkTrivialProducer.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>
#include <vtkTexture.h>
#include <vtkProperty.h>
#include <vtkOpenGLPolyDataMapper.h>

namespace imstk
{
VTKSurfaceMeshRenderDelegate::VTKSurfaceMeshRenderDelegate(std::shared_ptr<SurfaceMesh> surfaceMesh) :
    m_geometry(surfaceMesh),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    // Map vertices
    StdVectorOfVec3d& vertices = m_geometry->getVertexPositionsNotConst();
    double* vertData = reinterpret_cast<double*>(vertices.data());
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(vertData, vertices.size()*3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Copy cells
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType cell[3];
    for(const auto &t : m_geometry->getTrianglesVertices())
    {
        for(size_t i = 0; i < 3; ++i)
        {
            cell[i] = t[i];
        }
        cells->InsertNextCell(3,cell);
    }

    // Create PolyData
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetPolys(cells);

    // Create connection source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(polydata);
    m_geometry->m_dataModified = false;

    // Setup texture coordinates
    if (m_geometry->getDefaultTCoords() != "")
    {
        // Convert texture coordinates
        auto tcoords = m_geometry->getPointDataArray(m_geometry->getDefaultTCoords());
        if (tcoords == nullptr)
        {
            LOG(WARNING) << "No default texture coordinates array for geometry " << m_geometry;
        }
        else
        {
            auto vtkTCoords = vtkSmartPointer<vtkFloatArray>::New();
            vtkTCoords->SetNumberOfComponents(2);
            vtkTCoords->SetName(m_geometry->getDefaultTCoords().c_str());

            for (auto const tcoord : *tcoords)
            {
                double tuple[2] = { tcoord[0], tcoord[1] };
                vtkTCoords->InsertNextTuple(tuple);
            }

            polydata->GetPointData()->SetTCoords(vtkTCoords);
        }
    }

    // Update tangents
    if (m_geometry->getVertexTangents().size() > 0)
    {
        auto tangents = vtkSmartPointer<vtkFloatArray>::New();
        tangents->SetName("tangents");
        tangents->SetNumberOfComponents(3);

        for (auto const tangent : m_geometry->getVertexTangents())
        {
            float tempTangent[3] = {(float)tangent[0],
                                    (float)tangent[1],
                                    (float)tangent[2]};
            tangents->InsertNextTuple(tempTangent);
        }
        polydata->GetPointData()->AddArray(tangents);
    }

    // Update Transform, Render Properties
    this->update();
    this->setUpMapper(source->GetOutputPort(), false, m_geometry);
}

void
VTKSurfaceMeshRenderDelegate::updateDataSource()
{
    if (!m_geometry->m_dataModified)
    {
        return;
    }

    m_mappedVertexArray->Modified();

    m_geometry->m_dataModified = false;
}

void
VTKSurfaceMeshRenderDelegate::initializeTextures(TextureManager<VTKTextureDelegate>& textureManager)
{
    auto material = m_geometry->getRenderMaterial();
    if (material == nullptr)
    {
        return;
    }

    unsigned int currentUnit = 0;

    // Go through all of the textures
    for (int unit = 0; unit < Texture::Type::NONE; unit++)
    {
        // Get imstk texture
        auto texture = material->getTexture((Texture::Type)unit);
        if (texture->getPath() == "")
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
        m_actor->GetProperty()->SetTexture(currentUnit, textureDelegate->getTexture());
        currentUnit++;
    }
}

std::shared_ptr<Geometry>
VTKSurfaceMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}
} // imstk
