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

#include <vtkTrivialProducer.h>
#include <vtkDoubleArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkVersion.h>

namespace imstk
{
VTKPointSetRenderDelegate::VTKPointSetRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_visualModel = visualModel;

    auto geometry = std::static_pointer_cast<PointSet>(visualModel->getGeometry());

    // Map vertices
    StdVectorOfVec3d& vertices = geometry->getVertexPositionsNotConst();
    double* vertData = reinterpret_cast<double*>(vertices.data());
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(vertData, vertices.size()*3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Create PolyData
    auto pointsPolydata = vtkSmartPointer<vtkPolyData>::New();
    pointsPolydata->SetPoints(points);

    auto sphere = vtkSmartPointer<vtkSphereSource>::New();

    auto glyph = vtkSmartPointer<vtkGlyph3D>::New();

#if VTK_MAJOR_VERSION <= 5
    glyph->SetSource(sphere->GetOutput());
    glyph->SetInput(selectPoints->GetOutput());
#else
    glyph->SetSourceConnection(sphere->GetOutputPort());
    glyph->SetInputData(pointsPolydata);
#endif
    geometry->m_dataModified = false;

    // Update Transform, Render Properties
    this->update();

    // Setup custom mapper
    m_mapper->SetInputConnection(glyph->GetOutputPort());
    auto mapper = VTKCustomPolyDataMapper::SafeDownCast(m_mapper.GetPointer());
    if (!m_visualModel->getRenderMaterial())
    {
        m_visualModel->setRenderMaterial(std::make_shared<RenderMaterial>());
    }
    sphere->SetRadius(m_visualModel->getRenderMaterial()->getSphereGlyphSize());

    mapper->setRenderMaterial(m_visualModel->getRenderMaterial());
}

void
VTKPointSetRenderDelegate::updateDataSource()
{
    auto geometry = std::static_pointer_cast<PointSet>(m_visualModel->getGeometry());

    if (geometry->m_dataModified)
    {
        m_mappedVertexArray->Modified();
        geometry->m_dataModified = false;
    }
}
} // imstk
