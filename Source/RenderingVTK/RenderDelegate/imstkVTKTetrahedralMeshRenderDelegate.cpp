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
#include "imstkRenderMaterial.h"
#include "imstkGeometryUtilities.h"

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkTransform.h>

namespace imstk
{
VTKTetrahedralMeshRenderDelegate::VTKTetrahedralMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel),
    m_mesh(vtkSmartPointer<vtkUnstructuredGrid>::New()),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New()),
    m_cellArray(vtkSmartPointer<vtkCellArray>::New())
{
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(visualModel->getGeometry());

    // Create vtkUnstructuredGrid vtkPoints
    m_mappedVertexArray->SetNumberOfComponents(3);
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(0);
    points->SetData(m_mappedVertexArray);
    m_mesh->SetPoints(points);

    setVertexBuffer(geometry->getVertexPositions());

    setIndexBuffer(geometry->getCells());
    m_mesh->SetCells(VTK_TETRA, m_cellArray);

    // Map vertex scalars if it has them
    if (geometry->getVertexScalars() != nullptr)
    {
        m_mappedVertexScalarArray = GeometryUtils::coupleVtkDataArray(geometry->getVertexScalars());
        m_mesh->GetPointData()->SetScalars(m_mappedVertexScalarArray);
    }

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(geometry, &Geometry::modified, this, &VTKTetrahedralMeshRenderDelegate::geometryModified);

    // Setup the mapper
    {
        vtkNew<vtkDataSetMapper> mapper;
        mapper->SetInputData(m_mesh);
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetUserTransform(m_transform);
        m_actor  = actor;
        m_mapper = mapper;
    }

    update();
    updateRenderProperties();
}

void
VTKTetrahedralMeshRenderDelegate::processEvents()
{
    // Custom handling of events
    std::shared_ptr<TetrahedralMesh>         geom     = std::dynamic_pointer_cast<TetrahedralMesh>(m_visualModel->getGeometry());
    std::shared_ptr<VecDataArray<double, 3>> vertices = geom->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 4>>    indices  = geom->getCells();

    // Only use the most recent event from respective sender
    std::array<Command, 5> cmds;
    std::array<bool, 5>    contains = { false, false, false, false, false };
    rforeachEvent([&](Command cmd)
        {
            if (cmd.m_event->m_sender == m_visualModel.get() && !contains[0])
            {
                cmds[0]     = cmd;
                contains[0] = true;
            }
            else if (cmd.m_event->m_sender == m_material.get() && !contains[1])
            {
                cmds[1]     = cmd;
                contains[1] = true;
            }
            else if (cmd.m_event->m_sender == geom.get() && !contains[2])
            {
                cmds[2]     = cmd;
                contains[2] = true;
            }
            else if (cmd.m_event->m_sender == vertices.get() && !contains[3])
            {
                cmds[3]     = cmd;
                contains[3] = true;
            }
            else if (cmd.m_event->m_sender == indices.get() && !contains[4])
            {
                cmds[4]     = cmd;
                contains[4] = true;
            }
        });

    cmds[0].invoke();
    cmds[1].invoke();
    cmds[3].invoke();
    cmds[4].invoke();
    cmds[2].invoke(); // Process geometry changes last
}

void
VTKTetrahedralMeshRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(m_visualModel->getGeometry());
    setVertexBuffer(geometry->getVertexPositions());
}

void
VTKTetrahedralMeshRenderDelegate::indexDataModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(m_visualModel->getGeometry());
    setIndexBuffer(geometry->getCells());
}

void
VTKTetrahedralMeshRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(m_visualModel->getGeometry());

    // If the vertices were reallocated
    if (m_vertices != geometry->getVertexPositions())
    {
        setVertexBuffer(geometry->getVertexPositions());
    }

    // Assume vertices are always changed
    m_mappedVertexArray->Modified();

    // Only update index buffer when reallocated
    if (m_indices != geometry->getCells())
    {
        setIndexBuffer(geometry->getCells());
    }
}

void
VTKTetrahedralMeshRenderDelegate::setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices)
{
    // If the buffer changed
    if (m_vertices != vertices)
    {
        // If previous buffer exist
        if (m_vertices != nullptr)
        {
            // stop observing its changes
            disconnect(m_vertices, this, &VecDataArray<double, 3>::modified);
        }
        // Set new buffer and observe
        m_vertices = vertices;
        queueConnect<Event>(m_vertices, &VecDataArray<double, 3>::modified, this, &VTKTetrahedralMeshRenderDelegate::vertexDataModified);
    }

    // Couple the buffer
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    m_mappedVertexArray->Modified();
    m_mesh->GetPoints()->SetNumberOfPoints(m_vertices->size());
}

void
VTKTetrahedralMeshRenderDelegate::setIndexBuffer(std::shared_ptr<VecDataArray<int, 4>> indices)
{
    // If the buffer changed
    if (m_indices != indices)
    {
        // If previous buffer exist
        if (m_indices != nullptr)
        {
            // stop observing its changes
            disconnect(m_indices, this, &VecDataArray<int, 4>::modified);
        }
        // Set new buffer and observe
        m_indices = indices;
        queueConnect<Event>(m_indices, &VecDataArray<int, 4>::modified, this, &VTKTetrahedralMeshRenderDelegate::indexDataModified);
    }

    // Copy the buffer
    m_cellArray->Reset();
    vtkIdType cell[4];
    for (const auto& t : *m_indices)
    {
        for (size_t i = 0; i < 4; i++)
        {
            cell[i] = t[i];
        }
        m_cellArray->InsertNextCell(4, cell);
    }
    m_mesh->SetCells(VTK_TETRA, m_cellArray);
    m_cellArray->Modified();
    m_mesh->Modified();
}
} // namespace imstk