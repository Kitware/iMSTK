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
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkUnstructuredGrid.h>

namespace imstk
{
VTKTetrahedralMeshRenderDelegate::VTKTetrahedralMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_visualModel = visualModel;

    auto geometry = std::static_pointer_cast<TetrahedralMesh>(visualModel->getGeometry());

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
    vtkIdType cell[4];
    for (const auto& t : geometry->getTetrahedraVertices())
    {
        for (size_t i = 0; i < 4; ++i)
        {
            cell[i] = t[i];
        }
        cells->InsertNextCell(4, cell);
    }

    // Create Unstructured Grid
    m_mesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
    m_mesh->SetPoints(points);
    m_mesh->SetCells(VTK_TETRA, cells);

    // Mapper & Actor
    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(m_mesh);

    // Actor
    m_actor->SetMapper(mapper);

    // Update Transform, Render Properties
    this->update();
}

void
VTKTetrahedralMeshRenderDelegate::updateDataSource()
{
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(m_visualModel->getGeometry());

    if (geometry->m_dataModified)
    {
        m_mappedVertexArray->Modified();
        geometry->m_dataModified = false;
    }

    if (geometry->getTopologyChangedFlag())
    {
        m_mappedVertexArray->Modified(); /// \todo only modify if vertices change

        // Copy cells
        auto& maskedTets = std::dynamic_pointer_cast<TetrahedralMesh>(geometry)->getRemovedTetrahedra();

        auto      cells = vtkSmartPointer<vtkCellArray>::New();
        vtkIdType cell[4];
        size_t    tetId = 0;

        // Assign new cells
        for (const auto& t : geometry->getTetrahedraVertices())
        {
            if (!maskedTets[tetId])
            {
                for (size_t i = 0; i < 4; ++i)
                {
                    cell[i] = t[i];
                }
                cells->InsertNextCell(4, cell);
            }

            tetId++;
        }
        m_mesh->SetCells(VTK_TETRA, cells);
        geometry->setTopologyChangedFlag(false);
    }
}
} // imstk
