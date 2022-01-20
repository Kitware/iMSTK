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

#include "imstkVTKSurfaceNormalRenderDelegate.h"
#include "imstkPointSet.h"
#include "imstkVisualModel.h"
#include "imstkGeometryUtilities.h"
#include "imstkRenderMaterial.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkActor.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkTransform.h>
#include <vtkArrowSource.h>
#include <vtkOpenGLGlyph3DMapper.h>

namespace imstk
{
VTKSurfaceNormalRenderDelegate::VTKSurfaceNormalRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel),
    m_polydata(vtkSmartPointer<vtkPolyData>::New())
{
    auto surfMesh = std::static_pointer_cast<SurfaceMesh>(visualModel->getGeometry());
    m_surfMeshVertices = surfMesh->getVertexPositions();
    m_surfMeshIndices  = surfMesh->getTriangleIndices();

    // Compute the centers of the triangle
    m_triangleCenterVertices = computeTriangleCenters(m_surfMeshVertices, m_surfMeshIndices);
    m_triangleNormals = computeTriangleNormals(m_surfMeshVertices, m_surfMeshIndices);

    // Map vertices to VTK point data
    if (m_surfMeshVertices != nullptr)
    {
        m_mappedVertexArray = GeometryUtils::coupleVtkDataArray(m_triangleCenterVertices);
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(m_triangleCenterVertices->size());
        points->SetData(m_mappedVertexArray);
        m_polydata->SetPoints(points);
    }

    m_mappedNormalsArray = GeometryUtils::coupleVtkDataArray(m_triangleNormals);
    m_mappedNormalsArray->SetName("ImageScalars");
    m_polydata->GetPointData()->SetVectors(m_mappedNormalsArray);

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(surfMesh, &Geometry::modified, this, &VTKSurfaceNormalRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(m_surfMeshVertices, &AbstractDataArray::modified, this, &VTKSurfaceNormalRenderDelegate::vertexDataModified);

    // Setup mapper
    {
        vtkNew<vtkArrowSource> arrowSource;
        arrowSource->Update();
        m_glyphPolyData = arrowSource->GetOutput();
        vtkNew<vtkOpenGLGlyph3DMapper> mapper;
        mapper->OrientOn();
        mapper->SetInputData(m_polydata);
        mapper->SetSourceData(m_glyphPolyData);
        mapper->SetOrientationArray(m_mappedNormalsArray->GetName());
        mapper->ScalingOn();
        mapper->SetScaleFactor(visualModel->getRenderMaterial()->getPointSize());
        mapper->Update();
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = actor;
    }

    update();
    updateRenderProperties();
}

void
VTKSurfaceNormalRenderDelegate::processEvents()
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
VTKSurfaceNormalRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());
    m_surfMeshVertices = geometry->getVertexPositions();

    // Compute the centers of the triangle
    m_triangleCenterVertices = computeTriangleCenters(m_surfMeshVertices, m_surfMeshIndices);
    m_triangleNormals = computeTriangleNormals(m_surfMeshVertices, m_surfMeshIndices);

    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetVoidArray(reinterpret_cast<double*>(m_triangleCenterVertices->getPointer()), m_triangleCenterVertices->size() * 3, 1);
    m_mappedVertexArray->Modified();

    m_mappedNormalsArray->SetNumberOfComponents(3);
    m_mappedNormalsArray->SetVoidArray(reinterpret_cast<double*>(m_triangleNormals->getPointer()), m_triangleNormals->size() * 3, 1);
    m_mappedNormalsArray->Modified();
}

void
VTKSurfaceNormalRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    // Called when the geometry posts modified
    auto geometry = std::static_pointer_cast<PointSet>(m_visualModel->getGeometry());

    // Test if the vertex buffer changed
    //if (m_surfMeshVertices != geometry->getVertexPositions())
    {
        vertexDataModified(nullptr);
    }
}

void
VTKSurfaceNormalRenderDelegate::updateRenderProperties()
{
    VTKPolyDataRenderDelegate::updateRenderProperties();

    vtkOpenGLGlyph3DMapper::SafeDownCast(m_mapper)->SetScaleFactor(m_visualModel->getRenderMaterial()->getPointSize());
}

std::shared_ptr<VecDataArray<double, 3>>
VTKSurfaceNormalRenderDelegate::computeTriangleCenters(
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr,
    std::shared_ptr<VecDataArray<int, 3>> indicesPtr)
{
    auto                     newVerticesPtr = std::make_shared<VecDataArray<double, 3>>(indicesPtr->size());
    VecDataArray<double, 3>& newVertices    = *newVerticesPtr;

    VecDataArray<double, 3>& vertices = *verticesPtr;
    VecDataArray<int, 3>&    indices  = *indicesPtr;
    const double             ratio    = 1.0 / 3.0;
    for (int i = 0; i < indices.size(); i++)
    {
        const Vec3d& a = vertices[indices[i][0]];
        const Vec3d& b = vertices[indices[i][1]];
        const Vec3d& c = vertices[indices[i][2]];

        newVertices[i] = (a + b + c) * ratio;
    }
    return newVerticesPtr;
}

std::shared_ptr<VecDataArray<double, 3>>
VTKSurfaceNormalRenderDelegate::computeTriangleNormals(
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr,
    std::shared_ptr<VecDataArray<int, 3>> indicesPtr)
{
    auto                     orientationsPtr = std::make_shared<VecDataArray<double, 3>>(indicesPtr->size());
    VecDataArray<double, 3>& orientations    = *orientationsPtr;

    VecDataArray<double, 3>& vertices = *verticesPtr;
    VecDataArray<int, 3>&    indices  = *indicesPtr;
    for (int i = 0; i < indices.size(); i++)
    {
        const Vec3d& a = vertices[indices[i][0]];
        const Vec3d& b = vertices[indices[i][1]];
        const Vec3d& c = vertices[indices[i][2]];

        orientations[i] = (c - a).cross(c - b).normalized();
    }
    return orientationsPtr;
}
}