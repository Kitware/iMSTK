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

#include "imstkVTKdebugPointsRenderDelegate.h"

#include "imstkSurfaceMesh.h"

#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkTrivialProducer.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyLine.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkOpenGLPolyDataMapper.h>

namespace imstk
{
VTKdbgPointsRenderDelegate::VTKdbgPointsRenderDelegate(std::shared_ptr<DebugRenderPoints> vertices) :
    m_points(vertices),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    // Map vertices
    StdVectorOfVec3d& triVertData = vertices->getVertexPositionsNonConst();
    m_mappedVertexArray->SetNumberOfComponents(3);
    double* vertData = reinterpret_cast<double*>(triVertData.data());
    m_mappedVertexArray->SetArray(vertData, triVertData.size() * 3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(triVertData.size());
    points->SetData(m_mappedVertexArray);

    // Create PolyData
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);

    auto glyph = vtkSmartPointer<vtkPolyData>::New();    
    auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();

    auto glyph3D = vtkSmartPointer<vtkGlyph3D>::New();
    glyph3D->SetSourceConnection(sphereSource->GetOutputPort());
    glyph3D->SetInputData(polydata);
    glyph3D->Update();    

    m_points->setDataModifiedFlag(false);

    // Update Transform, Render Properties
    this->update();
    this->setUpMapper(glyph3D->GetOutputPort(), false, m_points->getRenderMaterial());
}

void
VTKdbgPointsRenderDelegate::updateDataSource()
{
    if (m_points->isModified())
    {
        m_mappedVertexArray->Modified();
    }
}
} // imstk