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

#include "imstkVTKFluidRenderDelegate.h"
#include "imstkPointSet.h"
#include "imstkVisualModel.h"
#include "imstkRenderMaterial.h"
#include "imstkGeometryUtilities.h"

#include <vtkOpenGLFluidMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkDoubleArray.h>
#include <vtkVolume.h>

namespace imstk
{
VTKFluidRenderDelegate::VTKFluidRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKVolumeRenderDelegate(visualModel),
    m_polydata(vtkSmartPointer<vtkPolyData>::New()),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    auto geometry = std::static_pointer_cast<PointSet>(visualModel->getGeometry());
    m_vertices = geometry->getVertexPositions();

    // Map vertices
    if (m_vertices != nullptr)
    {
        m_mappedVertexArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_vertices));
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(geometry->getNumVertices());
        points->SetData(m_mappedVertexArray);
        m_polydata->SetPoints(points);
    }

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(geometry, &Geometry::modified, this, &VTKFluidRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(geometry->getVertexPositions(), &VecDataArray<double, 3>::modified, this, &VTKFluidRenderDelegate::vertexDataModified);

    // Setup mapper
    {
        vtkNew<vtkOpenGLFluidMapper> mapper;
        mapper->SetInputData(m_polydata);
        vtkNew<vtkVolume> volume;
        volume->SetMapper(mapper);
        m_mapper = mapper;
        m_actor  = volume;
    }

    update();
    updateRenderProperties();
}

void
VTKFluidRenderDelegate::processEvents()
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
VTKFluidRenderDelegate::geometryModified(Event* imstkNotUsed(e))
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

void
VTKFluidRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
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
VTKFluidRenderDelegate::updateRenderProperties()
{
    vtkSmartPointer<vtkOpenGLFluidMapper> mapper = vtkOpenGLFluidMapper::SafeDownCast(m_mapper);

    // todo: Expose in RenderMaterial or subclass
    mapper->SetParticleRadius(m_material->getPointSize());
    mapper->SetSurfaceFilterIterations(3);
    mapper->SetSurfaceFilterRadius(5);
    mapper->SetSurfaceFilterMethod(vtkOpenGLFluidMapper::FluidSurfaceFilterMethod::NarrowRange);
    mapper->SetDisplayMode(vtkOpenGLFluidMapper::FluidDisplayMode::TransparentFluidVolume);
    mapper->SetAttenuationColor(0.1f, 0.9f, 0.9f);
    mapper->SetAttenuationScale(16.0f);
    mapper->SetOpaqueColor(0.9f, 0.1f, 0.1f);
    mapper->SetParticleColorPower(0.1f);
    mapper->SetParticleColorScale(0.57f);
    mapper->SetAdditionalReflection(0.0f);
    mapper->SetRefractiveIndex(1.5f);
    mapper->SetRefractionScale(0.07f);
}
} // imstk
