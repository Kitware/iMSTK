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

#include "imstkSurfaceMeshRenderDelegate.h"

#include <vtkTrivialProducer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>
#include <vtkTexture.h>
#include <vtkProperty.h>

#include "g3log/g3log.hpp"

namespace imstk
{

SurfaceMeshRenderDelegate::SurfaceMeshRenderDelegate(std::shared_ptr<SurfaceMesh> surfaceMesh) :
    m_geometry(surfaceMesh)
{
    // Map vertices
    m_mappedVertexArray = vtkSmartPointer<MappedVertexArray>::New();
    m_mappedVertexArray->SetVertexArray(m_geometry->getVerticesPositionsNotConst());

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Copy triangles
    auto triangles = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType cell[3];
    for(const auto &t : m_geometry->getTrianglesVertices())
    {
        cell[0] = t[0];
        cell[1] = t[1];
        cell[2] = t[2];
        triangles->InsertNextCell(3,cell);
    }

    // Create PolyData
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetPolys(triangles);

    // Mapper
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polydata);

    // Copy textures
    int unit = 0;
    auto readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();
    for (auto const &texturePair : surfaceMesh->getTextureMap())
    {
        std::string tCoordsName = texturePair.first;
        std::string tFileName = texturePair.second;

        // Convert texture coordinates
        auto tcoords = surfaceMesh->getPointDataArray(tCoordsName);
        auto vtkTCoords = vtkSmartPointer<vtkFloatArray>::New();
        vtkTCoords->SetNumberOfComponents(2);
        vtkTCoords->SetName(tCoordsName.c_str());
        for (auto const tcoord : tcoords)
        {
            double tuple[2] = {tcoord[0], tcoord[1]};
            vtkTCoords->InsertNextTuple(tuple);
        }
        polydata->GetPointData()->AddArray(vtkTCoords);

        // Read texture image
        auto imgReader = readerFactory->CreateImageReader2(tFileName.c_str());
        if (!imgReader)
        {
            LOG(WARNING) << "Could not find reader for " << tFileName;
            continue;
        }
        imgReader->SetFileName(tFileName.c_str());
        imgReader->Update();
        auto texture = vtkSmartPointer<vtkTexture>::New();
        texture->SetInputConnection(imgReader->GetOutputPort());
        texture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_ADD);
        texture->SetWrapMode(vtkTexture::VTKTextureWrapMode::ClampToBorder);

        // Link textures
        mapper->MapDataArrayToMultiTextureAttribute(unit, tCoordsName.c_str(),
                    vtkDataObject::FIELD_ASSOCIATION_POINTS);
        m_actor->GetProperty()->SetTexture(unit, texture);
        unit++;
    }

    // Actor
    m_actor->SetMapper(mapper);

    // Transform
    this->updateActorTransform();
}

void
SurfaceMeshRenderDelegate::update()
{
    // Base class update
    RenderDelegate::update();

    // TODO: only when vertices modified
    m_mappedVertexArray->Modified();
}

std::shared_ptr<Geometry>
SurfaceMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}

} // imstk
