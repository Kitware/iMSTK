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

#include "g3log/g3log.hpp"

namespace imstk
{

VTKLineMeshRenderDelegate::VTKLineMeshRenderDelegate(std::shared_ptr<LineMesh> lineMesh) :
    m_geometry(lineMesh)
{
    // Map vertices
    StdVectorOfVec3d& vertices = m_geometry->getVerticesPositionsNotConst();
    double* vertData = reinterpret_cast<double*>(vertices.data());
    m_mappedVertexArray = vtkSmartPointer<vtkDoubleArray>::New();
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(vertData, vertices.size()*3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Create line
    auto lines = vtkSmartPointer<vtkLineSource>::New();
    lines->SetPoints(points);

    // Setup Mapper & Actor
    this->setUpMapper(lines->GetOutputPort(), true);
    this->updateActorTransform();
}

void
VTKLineMeshRenderDelegate::update()
{
    // Base class update
    VTKRenderDelegate::update();

    m_mappedVertexArray->Modified(); // TODO: only modify if vertices change
}

std::shared_ptr<Geometry>
VTKLineMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}

} // imstk
