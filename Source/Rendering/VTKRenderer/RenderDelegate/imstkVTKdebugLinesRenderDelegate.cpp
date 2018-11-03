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

#include "imstkVTKdebugLinesRenderDelegate.h"

#include "imstkSurfaceMesh.h"

#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyLine.h>
#include <vtkProperty.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkTrivialProducer.h>

namespace imstk
{
VTKdbgLinesRenderDelegate::VTKdbgLinesRenderDelegate(std::shared_ptr<DebugRenderLines> renderLines) :
    m_Lines(renderLines),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    // Map vertices
    StdVectorOfVec3d& triVertData = renderLines->getVertexPositionsNonConst();
    m_mappedVertexArray->SetNumberOfComponents(3);
    double* vertData = reinterpret_cast<double*>(triVertData.data());
    m_mappedVertexArray->SetArray(vertData, triVertData.size() * 3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(triVertData.size());
    points->SetData(m_mappedVertexArray);

    // Create cells
    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds(triVertData.size());
    for (unsigned int i = 0; i < triVertData.size(); ++i)
    {
        polyLine->GetPointIds()->SetId(i, i);
    }
    auto cells = vtkSmartPointer<vtkCellArray>::New();
    cells->InsertNextCell(polyLine);

    // Create PolyData
    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetLines(cells);

    // Create connection source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(polydata);
    m_Lines->setDataModifiedFlag(false);

    // Update Transform, Render Properties
    this->updateDataSource();
    this->updateActorProperties();


    this->setUpMapper(source->GetOutputPort(), false, m_Lines->getRenderMaterial());
}

void
VTKdbgLinesRenderDelegate::updateDataSource()
{
    if (m_Lines->isModified())
    {
        m_mappedVertexArray->Modified();
    }
}

} // imstk