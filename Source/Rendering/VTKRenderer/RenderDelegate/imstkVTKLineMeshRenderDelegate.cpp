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

#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkLine.h>
#include <vtkTrivialProducer.h>
#include <vtkUnsignedCharArray.h>

namespace imstk
{
VTKLineMeshRenderDelegate::VTKLineMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_visualModel = visualModel;

    auto geometry = std::static_pointer_cast<LineMesh>(visualModel->getGeometry());

    // Map vertices
    StdVectorOfVec3d& vertices = geometry->getVertexPositionsNotConst();
    double*           vertData = reinterpret_cast<double*>(vertices.data());
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(vertData, vertices.size() * 3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Create index
    auto lineIndices = vtkSmartPointer<vtkCellArray>::New();

    for (auto line : geometry->getLinesVertices())
    {
        auto l = vtkSmartPointer<vtkLine>::New();
        l->GetPointIds()->SetId(0, line[0]);
        l->GetPointIds()->SetId(1, line[1]);
        lineIndices->InsertNextCell(l);
    }

    // Create line
    auto lines = vtkSmartPointer<vtkPolyData>::New();
    lines->SetPoints(points);
    lines->SetLines(lineIndices);

    // Add colors
    if (geometry->getVertexColors().size() == geometry->getNumVertices())
    {
        auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
        colors->SetNumberOfComponents(3);
        colors->SetName("Colors");

        for (auto color : geometry->getVertexColors())
        {
            unsigned char c[3] = { (unsigned char)(color.r * 255),
                                   (unsigned char)(color.g * 255),
                                   (unsigned char)(color.b * 255) };
            colors->InsertNextTypedTuple(c);
        }

        lines->GetPointData()->SetScalars(colors);
    }

    // Create connection source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(lines);
    geometry->m_dataModified = false;

    // Update Transform, Render Properties
    this->update();

    // Setup Mapper & Actor
    this->setUpMapper(source->GetOutputPort(), true, m_visualModel->getRenderMaterial());
}

void
VTKLineMeshRenderDelegate::updateDataSource()
{
    auto geometry = std::static_pointer_cast<LineMesh>(m_visualModel->getGeometry());

    if (!geometry->m_dataModified)
    {
        return;
    }

    m_mappedVertexArray->Modified();

    geometry->m_dataModified = false;
}
} // imstk
