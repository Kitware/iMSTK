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

#include "imstkVTKTetrahedralMeshRenderDelegate.h"

#include "imstkVTKMappedVertexArray.h"

#include <vtkCellArray.h>
#include <vtkTrivialProducer.h>
#include <vtkDataSetMapper.h>

#include "g3log/g3log.hpp"

namespace imstk
{

VTKTetrahedralMeshRenderDelegate::VTKTetrahedralMeshRenderDelegate(std::shared_ptr<TetrahedralMesh> tetrahedralMesh) :
    m_geometry(tetrahedralMesh)
{
    const size_t dim = 4;

    // Map vertices
    auto mappedVertexArray = vtkSmartPointer<VTKMappedVertexArray>::New();
    mappedVertexArray->SetVertexArray(m_geometry->getVerticesPositionsNotConst());

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_geometry->getNumVertices());
    points->SetData(mappedVertexArray);

    // Copy cells
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType cell[dim];
    for(const auto &t : m_geometry->getTetrahedraVertices())
    {
        for(size_t i = 0; i < dim; ++i)
        {
            cell[i] = t[i];
        }
        cells->InsertNextCell(dim,cell);
    }

    // Create PolyData
    auto unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    unstructuredGrid->SetPoints(points);
    unstructuredGrid->SetCells(VTK_TETRA, cells);

    // Create Source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(unstructuredGrid);

    // Set up mapper
    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputConnection(source->GetOutputPort());
    m_actor->SetMapper(mapper);

    this->updateActorTransform();
}

std::shared_ptr<Geometry>
VTKTetrahedralMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}

} // imstk
