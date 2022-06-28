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

#include "imstkVTKVertexLabelRenderDelegate.h"
#include "imstkGeometryUtilities.h"
#include "imstkPointSet.h"
#include "imstkRenderMaterial.h"
#include "imstkVertexLabelVisualModel.h"

#include <vtkActor2D.h>
#include <vtkDoubleArray.h>
#include <vtkLabeledDataMapper.h>
#include <vtkPolyData.h>
#include <vtkTextProperty.h>

namespace imstk
{
VTKVertexLabelRenderDelegate::VTKVertexLabelRenderDelegate(
    std::shared_ptr<VisualModel> visualModel) : VTKRenderDelegate(visualModel),
    m_polydata(vtkSmartPointer<vtkPolyData>::New()),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    auto vertexLabelVisualModel = std::dynamic_pointer_cast<VertexLabelVisualModel>(visualModel);
    m_geometry = std::dynamic_pointer_cast<PointSet>(visualModel->getGeometry());
    CHECK(m_geometry != nullptr) << "VTKvertexLabelRenderDelegate only works with PointSet geometry";

    // Get our own handles to these in case the geometry changes them
    m_vertices = m_geometry->getVertexPositions();

    // Map vertices to VTK point data
    if (m_vertices != nullptr)
    {
        m_mappedVertexArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_vertices));
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(m_geometry->getNumVertices());
        points->SetData(m_mappedVertexArray);
        m_polydata->SetPoints(points);
    }

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(m_geometry, &Geometry::modified, this,
        &VTKVertexLabelRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(m_geometry->getVertexPositions(), &VecDataArray<double, 3>::modified, this,
        &VTKVertexLabelRenderDelegate::vertexDataModified);

    // Setup mapper
    // \todo: Replace with vtkFastLabelMapper on next VTK upgrade
    {
        vtkNew<vtkLabeledDataMapper> mapper;
        mapper->SetInputData(m_polydata);
        mapper->SetLabelModeToLabelIds();

        vtkNew<vtkActor2D> actor;
        actor->SetMapper(mapper);
        m_mapper = mapper;
        m_actor  = actor;
    }

    update();
    updateRenderProperties();
}

void
VTKVertexLabelRenderDelegate::processEvents()
{
    // Custom handling of events
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = m_geometry->getVertexPositions();

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
        });

    // Now do all the commands
    cmds[0].invoke(); // Update VisualModel
    cmds[1].invoke(); // Update RenderMaterial
    cmds[3].invoke(); // Update vertices
    cmds[2].invoke(); // Update geometry as a whole
}

void
VTKVertexLabelRenderDelegate::updateRenderProperties()
{
    auto visualModel = std::dynamic_pointer_cast<VertexLabelVisualModel>(m_visualModel);
    auto labelMapper = vtkLabeledDataMapper::SafeDownCast(m_mapper);

    labelMapper->SetLabelFormat(visualModel->getFormat().c_str());

    const Color& color = visualModel->getTextColor();
    labelMapper->GetLabelTextProperty()->SetColor(color.r, color.g, color.b);
    labelMapper->GetLabelTextProperty()->SetFontSize(visualModel->getFontSize());
    labelMapper->GetLabelTextProperty()->SetShadow(false);
}

void
VTKVertexLabelRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    setVertexBuffer(m_geometry->getVertexPositions());
}

void
VTKVertexLabelRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    // If the vertices were reallocated
    if (m_vertices != m_geometry->getVertexPositions())
    {
        setVertexBuffer(m_geometry->getVertexPositions());
    }

    // Assume vertices are always changed
    m_mappedVertexArray->Modified();
}

void
VTKVertexLabelRenderDelegate::setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices)
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
        queueConnect<Event>(m_vertices, &VecDataArray<double, 3>::modified, this,
            &VTKVertexLabelRenderDelegate::vertexDataModified);
    }

    // Couple the buffer
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    m_mappedVertexArray->Modified();
    m_polydata->GetPoints()->SetNumberOfPoints(m_vertices->size());
}
} // namespace imstk