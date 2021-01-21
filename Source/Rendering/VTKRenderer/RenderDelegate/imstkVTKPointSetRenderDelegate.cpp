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

#include "imstkVTKPointSetRenderDelegate.h"
#include "imstkPointSet.h"
#include "imstkVisualModel.h"
#include "imstkGeometryUtilities.h"
#include "imstkRenderMaterial.h"

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkActor.h>
#include <vtkTransform.h>

namespace imstk
{
VTKPointSetRenderDelegate::VTKPointSetRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel),
    m_polydata(vtkSmartPointer<vtkPolyData>::New()),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New()),
    m_mappedVertexScalarArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    auto geometry = std::static_pointer_cast<PointSet>(visualModel->getGeometry());
    m_vertices = geometry->getVertexPositions();

    // Map vertices to VTK point data
    if (m_vertices != nullptr)
    {
        m_mappedVertexArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_vertices));
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(geometry->getNumVertices());
        points->SetData(m_mappedVertexArray);
        m_polydata->SetPoints(points);
    }

    // Map vertex scalars if it has them
    if (geometry->getVertexScalars() != nullptr)
    {
        m_mappedVertexScalarArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(geometry->getVertexScalars()));
        m_polydata->GetPointData()->SetScalars(m_mappedVertexScalarArray);
    }

    // \todo: Slow, replace with opengl hardware instancing which is actually an OpenGLMapper
    vtkNew<vtkVertexGlyphFilter> glyphFilter;
    glyphFilter->SetInputData(m_polydata);
    glyphFilter->Update();

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(geometry, EventType::Modified, this, &VTKPointSetRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(geometry->getVertexPositions(), EventType::Modified, this, &VTKPointSetRenderDelegate::vertexDataModified);

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(glyphFilter->GetOutputPort());
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
VTKPointSetRenderDelegate::processEvents()
{
    // Custom handling of events
    std::shared_ptr<PointSet>                geom     = std::dynamic_pointer_cast<PointSet>(m_visualModel->getGeometry());
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
VTKPointSetRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<PointSet>(m_visualModel->getGeometry());
    m_vertices = geometry->getVertexPositions();
    if (m_vertices->getVoidPointer() != m_mappedVertexArray->GetVoidPointer(0))
    {
        m_mappedVertexArray->SetNumberOfComponents(3);
        m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    }
    m_mappedVertexArray->Modified();
}

void
VTKPointSetRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    // Called when the geometry posts modified
    auto geometry = std::static_pointer_cast<PointSet>(m_visualModel->getGeometry());

    // Test if the vertex buffer changed
    if (m_vertices != geometry->getVertexPositions())
    {
        //printf("Vertex data swapped\n");
        m_vertices = geometry->getVertexPositions();
        {
            // Update the pointer of the coupled array
            m_mappedVertexArray->SetNumberOfComponents(3);
            m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
        }
    }
    m_mappedVertexArray->Modified();
}
} // imstk
