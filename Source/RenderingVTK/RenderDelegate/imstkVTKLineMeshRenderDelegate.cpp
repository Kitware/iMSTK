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

#include "imstkVTKLineMeshRenderDelegate.h"
#include "imstkLineMesh.h"
#include "imstkVisualModel.h"
#include "imstkRenderMaterial.h"
#include "imstkGeometryUtilities.h"

#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkLineSource.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkPointData.h>
#include <vtkTransform.h>

namespace imstk
{
VTKLineMeshRenderDelegate::VTKLineMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel),
    m_polydata(vtkSmartPointer<vtkPolyData>::New()),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_geometry = std::static_pointer_cast<LineMesh>(visualModel->getGeometry());

    // Get our own handles to these in case the geometry changes them
    m_vertices = m_geometry->getVertexPositions();
    m_indices  = m_geometry->getLinesIndices();

    // Map vertices to VTK point data
    if (m_vertices != nullptr)
    {
        m_mappedVertexArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_vertices));
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(m_geometry->getNumVertices());
        points->SetData(m_mappedVertexArray);
        m_polydata->SetPoints(points);
    }

    // Map indices to VTK cell data (copied)
    if (m_indices != nullptr)
    {
        m_cellArray = vtkSmartPointer<vtkCellArray>::New();
        vtkIdType cell[2];
        for (const auto& t : *m_indices)
        {
            for (size_t i = 0; i < 2; i++)
            {
                cell[i] = t[i];
            }
            m_cellArray->InsertNextCell(2, cell);
        }
        m_polydata->SetLines(m_cellArray);
    }

    // Map vertex scalars if it has them
    if (m_geometry->getVertexScalars() != nullptr)
    {
        setVertexScalarBuffer(m_geometry->getVertexScalars());
    }

    // Map cell scalars if it has them
    if (m_geometry->getCellScalars() != nullptr)
    {
        setCellScalarBuffer(m_geometry->getCellScalars());
    }

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(m_geometry, &Geometry::modified, this, &VTKLineMeshRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(m_geometry->getVertexPositions(), &VecDataArray<double, 3>::modified, this, &VTKLineMeshRenderDelegate::vertexDataModified);

    // When index buffer internals are modified
    queueConnect<Event>(m_geometry->getLinesIndices(), &VecDataArray<int, 2>::modified, this, &VTKLineMeshRenderDelegate::indexDataModified);

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(m_polydata);
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = actor;
        if (auto glMapper = vtkOpenGLPolyDataMapper::SafeDownCast(m_mapper.GetPointer()))
        {
            glMapper->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);
        }
    }

    update();
    updateRenderProperties();
}

void
VTKLineMeshRenderDelegate::processEvents()
{
    // Custom handling of events
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr      = m_geometry->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 2>>    indicesPtr       = m_geometry->getLinesIndices();
    std::shared_ptr<AbstractDataArray>       cellScalarsPtr   = m_geometry->getCellScalars();
    std::shared_ptr<AbstractDataArray>       vertexScalarsPtr = m_geometry->getVertexScalars();

    // Only use the most recent event from respective sender
    std::array<Command, 7> cmds;
    std::array<bool, 7>    contains = { false, false, false, false, false, false, false };
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
            else if (cmd.m_event->m_sender == m_geometry.get() && !contains[2])
            {
                cmds[2]     = cmd;
                contains[2] = true;
            }
            else if (cmd.m_event->m_sender == verticesPtr.get() && !contains[3])
            {
                cmds[3]     = cmd;
                contains[3] = true;
            }
            else if (cmd.m_event->m_sender == cellScalarsPtr.get() && !contains[4])
            {
                cmds[4]     = cmd;
                contains[4] = true;
            }
            else if (cmd.m_event->m_sender == vertexScalarsPtr.get() && !contains[5])
            {
                cmds[5]     = cmd;
                contains[5] = true;
            }
            else if (cmd.m_event->m_sender == indicesPtr.get() && !contains[6])
            {
                cmds[6]     = cmd;
                contains[6] = true;
            }
        });

    // Now do all the commands
    cmds[0].invoke(); // Update VisualModel
    cmds[1].invoke(); // Update RenderMaterial
    cmds[3].invoke(); // Update vertices
    cmds[4].invoke(); // Update cell scalars
    cmds[5].invoke(); // Update vertex scalars
    cmds[6].invoke(); // Update indices
    cmds[2].invoke(); // Update geometry as a whole
}

void
VTKLineMeshRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    setVertexBuffer(m_geometry->getVertexPositions());
}

void
VTKLineMeshRenderDelegate::indexDataModified(Event* imstkNotUsed(e))
{
    setIndexBuffer(m_geometry->getLinesIndices());
}

void
VTKLineMeshRenderDelegate::vertexScalarsModified(Event* imstkNotUsed(e))
{
    setVertexScalarBuffer(m_geometry->getVertexScalars());
}

void
VTKLineMeshRenderDelegate::cellScalarsModified(Event* imstkNotUsed(e))
{
    setCellScalarBuffer(m_geometry->getCellScalars());
}

void
VTKLineMeshRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    // If the vertices were reallocated
    if (m_vertices != m_geometry->getVertexPositions())
    {
        setVertexBuffer(m_geometry->getVertexPositions());
    }

    // Assume vertices are always changed
    m_mappedVertexArray->Modified();

    // Only update index buffer when reallocated
    if (m_indices != m_geometry->getLinesIndices())
    {
        setIndexBuffer(m_geometry->getLinesIndices());
    }
    if (m_vertexScalars != m_geometry->getVertexScalars())
    {
        setVertexScalarBuffer(m_geometry->getVertexScalars());
    }
    if (m_cellScalars != m_geometry->getCellScalars())
    {
        setCellScalarBuffer(m_geometry->getCellScalars());
    }
}

void
VTKLineMeshRenderDelegate::setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices)
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
        queueConnect<Event>(m_vertices, &VecDataArray<double, 3>::modified, this, &VTKLineMeshRenderDelegate::vertexDataModified);
    }

    // Couple the buffer
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    m_mappedVertexArray->Modified();
    m_polydata->GetPoints()->SetNumberOfPoints(m_vertices->size());
}

void
VTKLineMeshRenderDelegate::setIndexBuffer(std::shared_ptr<VecDataArray<int, 2>> indices)
{
    // If the buffer changed
    if (m_indices != indices)
    {
        // If previous buffer exist
        if (m_indices != nullptr)
        {
            // stop observing its changes
            disconnect(m_indices, this, &VecDataArray<int, 2>::modified);
        }
        // Set new buffer and observe
        m_indices = indices;
        queueConnect<Event>(m_indices, &VecDataArray<int, 2>::modified, this, &VTKLineMeshRenderDelegate::indexDataModified);
    }

    // Copy the buffer
    // Copy cells
    m_cellArray->Reset();
    vtkIdType cell[3];
    for (const auto& t : *m_indices)
    {
        for (size_t i = 0; i < 2; i++)
        {
            cell[i] = t[i];
        }
        m_cellArray->InsertNextCell(2, cell);
    }
    m_cellArray->Modified();
}

void
VTKLineMeshRenderDelegate::setVertexScalarBuffer(std::shared_ptr<AbstractDataArray> scalars)
{
    // If the buffer changed
    if (m_vertexScalars != scalars)
    {
        // If previous buffer exist
        if (m_vertexScalars != nullptr)
        {
            // stop observing its changes
            disconnect(m_vertexScalars, this, &AbstractDataArray::modified);
        }
        // Set new buffer and observe
        m_vertexScalars = scalars;
        queueConnect<Event>(m_vertexScalars, &AbstractDataArray::modified, this, &VTKLineMeshRenderDelegate::vertexScalarsModified);
        m_mappedVertexScalarArray = GeometryUtils::coupleVtkDataArray(m_vertexScalars);
        m_polydata->GetPointData()->SetScalars(m_mappedVertexScalarArray);
    }
    m_mappedVertexScalarArray->SetNumberOfComponents(m_vertexScalars->getNumberOfComponents());
    m_mappedVertexScalarArray->SetVoidArray(m_vertexScalars->getVoidPointer(),
        static_cast<vtkIdType>(m_vertexScalars->size()), 1);
    m_mappedVertexScalarArray->Modified();
}

void
VTKLineMeshRenderDelegate::setCellScalarBuffer(std::shared_ptr<AbstractDataArray> scalars)
{
    // If the buffer changed
    if (m_cellScalars != scalars)
    {
        // If previous buffer exist
        if (m_cellScalars != nullptr)
        {
            // stop observing its changes
            disconnect(m_cellScalars, this, &AbstractDataArray::modified);
        }
        // Set new buffer and observe
        m_cellScalars = scalars;
        queueConnect<Event>(m_cellScalars, &AbstractDataArray::modified, this, &VTKLineMeshRenderDelegate::cellScalarsModified);
        m_mappedCellScalarArray = GeometryUtils::coupleVtkDataArray(m_cellScalars);
        m_polydata->GetCellData()->SetScalars(m_mappedCellScalarArray);
    }

    m_mappedCellScalarArray->SetNumberOfComponents(m_cellScalars->getNumberOfComponents());
    m_mappedCellScalarArray->SetVoidArray(m_cellScalars->getVoidPointer(),
        static_cast<vtkIdType>(m_cellScalars->size()), 1);
    m_mappedCellScalarArray->Modified();
}
} // namespace imstk