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
#include "imstkVecDataArray.h"
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
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(visualModel->getGeometry());
    m_vertices = geometry->getVertexPositions();
    m_indices  = geometry->getTetrahedraIndices();

    // Map vertices to VTK point data
    if (m_vertices != nullptr)
    {
        m_mappedVertexArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_vertices));
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(geometry->getNumVertices());
        points->SetData(m_mappedVertexArray);
        m_mesh->SetPoints(points);
    }

    // Map indices to VTK cell data (copied)
    {
        m_cellArray = vtkSmartPointer<vtkCellArray>::New();
        vtkIdType cell[4];
        for (const auto& t : *m_indices)
        {
            for (size_t i = 0; i < 4; ++i)
            {
                cell[i] = t[i];
            }
            m_cellArray->InsertNextCell(4, cell);
        }
        m_mesh->SetCells(VTK_TETRA, m_cellArray);
    }

    // Map vertex scalars if it has them
    if (geometry->getVertexScalars() != nullptr)
    {
        m_mappedVertexScalarArray = GeometryUtils::coupleVtkDataArray(geometry->getVertexScalars());
        m_mesh->GetPointData()->SetScalars(m_mappedVertexScalarArray);
    }

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(geometry, EventType::Modified, this, &VTKTetrahedralMeshRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(geometry->getVertexPositions(), EventType::Modified, this, &VTKTetrahedralMeshRenderDelegate::vertexDataModified);

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

    // Only use the most recent event from respective sender
    std::list<Command> cmds;
    bool               contains[4] = { false, false, false, false };
    rforeachEvent([&](Command cmd)
        {
            if (cmd.m_event->m_sender == m_visualModel.get() && !contains[0])
            {
                cmds.push_back(cmd);
                contains[0] = true;
            }
            else if (cmd.m_event->m_sender == m_material.get() && !contains[1])
            {
                cmds.push_back(cmd);
                contains[1] = true;
            }
            else if (cmd.m_event->m_sender == geom.get() && !contains[2])
            {
                cmds.push_back(cmd);
                contains[2] = true;
            }
            else if (cmd.m_event->m_sender == vertices.get() && !contains[3])
            {
                cmds.push_back(cmd);
                contains[3] = true;
            }
        });

    // Now do each event in order recieved
    for (std::list<Command>::reverse_iterator i = cmds.rbegin(); i != cmds.rend(); i++)
    {
        i->invoke();
    }
}

void
VTKTetrahedralMeshRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(m_visualModel->getGeometry());
    m_vertices = geometry->getVertexPositions();
    if (m_vertices->getVoidPointer() != m_mappedVertexArray->GetVoidPointer(0))
    {
        m_mappedVertexArray->SetNumberOfComponents(3);
        m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    }
    m_mappedVertexArray->Modified();
}

//void
//VTKTetrahedralMeshRenderDelegate::indexDataModified(Event* e)
//{
//
//}

void
VTKTetrahedralMeshRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    // Called when the geometry posts modified
    auto geometry = std::static_pointer_cast<TetrahedralMesh>(m_visualModel->getGeometry());

    // Test if the vertex buffer changed
    if (m_vertices != geometry->getVertexPositions())
    {
        //printf("Vertex data swapped\n");
        m_vertices = geometry->getVertexPositions();
        {
            // Update the pointer of the coupled array
            m_mappedVertexArray->SetNumberOfComponents(3);
            m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
            m_mappedVertexArray->Modified();
        }
    }

    // Test if the index buffer changed
    if (m_indices != geometry->getTetrahedraIndices())
    {
        //printf("Index data swapped\n");
        m_indices = geometry->getTetrahedraIndices();
        {
            // Copy cells
            m_cellArray = vtkSmartPointer<vtkCellArray>::New();
            vtkIdType cell[3];
            for (const auto& t : *m_indices)
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    cell[i] = t[i];
                }
                m_cellArray->InsertNextCell(3, cell);
            }
            m_mesh->SetCells(VTK_TETRA, m_cellArray);
            m_mesh->Modified();
        }
    }
}
} // imstk
