/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKPointSetRenderDelegate.h"
#include "imstkPointSet.h"
#include "imstkVisualModel.h"
#include "imstkGeometryUtilities.h"
#include "imstkRenderMaterial.h"

#include <array>
#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkPointData.h>
#include <vtkTransform.h>
#include <vtkVertexGlyphFilter.h>

namespace imstk
{
VTKPointSetRenderDelegate::VTKPointSetRenderDelegate() :
    m_geometry(nullptr),
    m_vertices(nullptr),
    m_vertexScalars(nullptr),
    m_polydata(vtkSmartPointer<vtkPolyData>::New()),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New()),
    m_mappedVertexScalarArray(vtkSmartPointer<vtkDoubleArray>::New())
{
}

void
VTKPointSetRenderDelegate::init()
{
    m_geometry = std::dynamic_pointer_cast<PointSet>(m_visualModel->getGeometry());
    CHECK(m_geometry != nullptr) << "VTKPointSetRenderDelegate only works with PointSet geometry";

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

    // Map vertex scalars if it has them
    if (m_geometry->getVertexScalars() != nullptr)
    {
        setVertexScalarBuffer(m_geometry->getVertexScalars());
    }

    // \todo: Slow, replace with opengl hardware instancing which is actually an OpenGLMapper
    vtkNew<vtkVertexGlyphFilter> glyphFilter;
    glyphFilter->SetInputData(m_polydata);
    glyphFilter->Update();

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(m_geometry, &Geometry::modified,
        std::static_pointer_cast<VTKPointSetRenderDelegate>(shared_from_this()),
        &VTKPointSetRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(m_geometry->getVertexPositions(), &VecDataArray<double, 3>::modified,
        std::static_pointer_cast<VTKPointSetRenderDelegate>(shared_from_this()),
        &VTKPointSetRenderDelegate::vertexDataModified);

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
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr      = m_geometry->getVertexPositions();
    std::shared_ptr<AbstractDataArray>       vertexScalarsPtr = m_geometry->getVertexScalars();

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
            else if (cmd.m_event->m_sender == vertexScalarsPtr.get() && !contains[4])
            {
                cmds[4]     = cmd;
                contains[4] = true;
            }
        });

    // Now do all the commands
    cmds[0].invoke(); // Update VisualModel
    cmds[1].invoke(); // Update RenderMaterial
    cmds[3].invoke(); // Update vertices
    cmds[4].invoke(); // Update vertex scalars
    cmds[2].invoke(); // Update geometry as a whole
}

void
VTKPointSetRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    setVertexBuffer(m_geometry->getVertexPositions());
}

void
VTKPointSetRenderDelegate::vertexScalarsModified(Event* imstkNotUsed(e))
{
    setVertexScalarBuffer(m_geometry->getVertexScalars());
}

void
VTKPointSetRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    // If the vertices were reallocated
    if (m_vertices != m_geometry->getVertexPositions())
    {
        setVertexBuffer(m_geometry->getVertexPositions());
    }

    // Assume vertices are always changed
    m_mappedVertexArray->Modified();

    if (m_vertexScalars != m_geometry->getVertexScalars())
    {
        setVertexScalarBuffer(m_geometry->getVertexScalars());
    }
}

void
VTKPointSetRenderDelegate::setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices)
{
    // If the buffer changed
    if (m_vertices != vertices)
    {
        // If previous buffer exist
        if (m_vertices != nullptr)
        {
            // stop observing its changes
            disconnect(m_vertices,
                std::static_pointer_cast<VTKPointSetRenderDelegate>(shared_from_this()),
                &VecDataArray<double, 3>::modified);
        }
        // Set new buffer and observe
        m_vertices = vertices;
        queueConnect<Event>(m_vertices, &VecDataArray<double, 3>::modified,
            std::static_pointer_cast<VTKPointSetRenderDelegate>(shared_from_this()),
            &VTKPointSetRenderDelegate::vertexDataModified);
    }

    // Couple the buffer
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    m_mappedVertexArray->Modified();
    m_polydata->GetPoints()->SetNumberOfPoints(m_vertices->size());
}

void
VTKPointSetRenderDelegate::setVertexScalarBuffer(std::shared_ptr<AbstractDataArray> scalars)
{
    // If the buffer changed
    if (m_vertexScalars != scalars)
    {
        // If previous buffer exist
        if (m_vertexScalars != nullptr)
        {
            // stop observing its changes
            disconnect(m_vertexScalars,
                std::static_pointer_cast<VTKPointSetRenderDelegate>(shared_from_this()),
                &AbstractDataArray::modified);
        }
        // Set new buffer and observe
        m_vertexScalars = scalars;
        queueConnect<Event>(m_vertexScalars, &AbstractDataArray::modified,
            std::static_pointer_cast<VTKPointSetRenderDelegate>(shared_from_this()),
            &VTKPointSetRenderDelegate::vertexScalarsModified);
        m_mappedVertexScalarArray = GeometryUtils::coupleVtkDataArray(m_vertexScalars);
        m_polydata->GetPointData()->SetScalars(m_mappedVertexScalarArray);
    }
    m_mappedVertexScalarArray->SetNumberOfComponents(m_vertexScalars->getNumberOfComponents());
    m_mappedVertexScalarArray->SetVoidArray(m_vertexScalars->getVoidPointer(),
        static_cast<vtkIdType>(m_vertexScalars->size()), 1);
    m_mappedVertexScalarArray->Modified();
}
} // namespace imstk