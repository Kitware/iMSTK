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

#include "imstkMappedVertexArray.h"

#include <vtkTrivialProducer.h>

#include "g3log/g3log.hpp"

namespace imstk {
SurfaceMeshRenderDelegate::SurfaceMeshRenderDelegate(std::shared_ptr<SurfaceMesh> surfaceMesh) :
    m_geometry(surfaceMesh)
{
    // Map vertices
    auto mappedVertexArray = vtkSmartPointer<MappedVertexArray>::New();
    mappedVertexArray->SetVertexArray(m_geometry->getVerticesPositionsNotConst());

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_geometry->getNumVertices());
    points->SetData(mappedVertexArray);

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

    // Create Source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(polydata);

    this->setActorMapper(source->GetOutputPort());
    this->updateActorTransform();
}

std::shared_ptr<Geometry>
SurfaceMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}
}
