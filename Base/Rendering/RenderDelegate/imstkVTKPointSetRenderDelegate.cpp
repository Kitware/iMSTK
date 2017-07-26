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

namespace imstk
{
VTKPointSetRenderDelegate::VTKPointSetRenderDelegate(std::shared_ptr<PointSet> mesh) :
    m_geometry(mesh),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    // Map vertices
    StdVectorOfVec3d& vertices = m_geometry->getVertexPositionsNotConst();
    double* vertData = reinterpret_cast<double*>(vertices.data());
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(vertData, vertices.size()*3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Create PolyData
    auto pointsPolydata = vtkSmartPointer<vtkPolyData>::New();
    pointsPolydata->SetPoints(points);

    auto vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
#if VTK_MAJOR_VERSION <= 5
    vertexFilter->SetInputConnection(pointsPolydata->GetProducerPort());
#else
    vertexFilter->SetInputData(pointsPolydata);
#endif
    vertexFilter->Update();
    m_geometry->m_dataModified = false;

    // Update Transform, Render Properties
    this->update();

    // Setup custom mapper
    m_mapper->SetInputConnection(vertexFilter->GetOutputPort());
    auto mapper = VTKCustomPolyDataMapper::SafeDownCast(m_mapper.GetPointer());
    if (!mesh->getRenderMaterial())
    {
        mesh->setRenderMaterial(std::make_shared<RenderMaterial>());
    }
    mapper->setGeometry(mesh);
}

void
VTKPointSetRenderDelegate::updateDataSource()
{
    if (m_geometry->m_dataModified)
    {
        m_mappedVertexArray->Modified();
        m_geometry->m_dataModified = false;
    }
}

std::shared_ptr<Geometry>
VTKPointSetRenderDelegate::getGeometry() const
{
    return m_geometry;
}
} // imstk
