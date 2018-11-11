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

#include "imstkVTKdebugTrianglesRenderDelegate.h"

#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkTrivialProducer.h>

namespace imstk
{
VTKdbgTrianglesRenderDelegate::VTKdbgTrianglesRenderDelegate(std::shared_ptr<DebugRenderTriangles> renderTriangles) :
    m_triangles(renderTriangles),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New()),
    VTKRenderDelegate()
{
    // Map vertices in memory
    StdVectorOfVec3d& triVertData = renderTriangles->getVertexPositionsNonConst();
    m_mappedVertexArray->SetNumberOfComponents(3);
    double* vertData = reinterpret_cast<double*>(triVertData.data());
    m_mappedVertexArray->SetArray(vertData, triVertData.size() * 3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(triVertData.size());
    points->SetData(m_mappedVertexArray);

    // Copy cells
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType cell[3];
    for (int i = 0; i < triVertData.size() / 3; ++i)
    {
        cell[0] = 3 * i;
        cell[1] = cell[0] + 1;
        cell[2] = cell[0] + 2;
        cells->InsertNextCell(3, cell);
    }

    // Create PolyData
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetPolys(cells);

    // Create connection source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(polydata);
    m_triangles->setDataModifiedFlag(false);

    // Update Transform, Render Properties
    //this->update();
    this->updateDataSource();
    this->updateActorProperties();
    this->setUpMapper(source->GetOutputPort(), false, m_triangles->getRenderMaterial());
}

void
VTKdbgTrianglesRenderDelegate::updateDataSource()
{
    if (m_triangles->isModified())
    {
        m_mappedVertexArray->Modified();
    }
}
} // imstk