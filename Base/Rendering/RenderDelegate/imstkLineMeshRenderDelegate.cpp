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

#include "imstkLineMeshRenderDelegate.h"

#include <vtkTrivialProducer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageReader2.h>
#include <vtkTexture.h>
#include <vtkProperty.h>


#include "g3log/g3log.hpp"

namespace imstk
{

LineMeshRenderDelegate::LineMeshRenderDelegate(std::shared_ptr<LineMesh> lineMesh) :
    m_geometry(lineMesh)
{
    // Map vertices
    m_mappedVertexArray = vtkDoubleArray::New();
    m_mappedVertexArray->SetNumberOfComponents(3);
    auto vertices = m_geometry->getVerticesPositionsNotConst();
    for (int i = 0; i < vertices.size(); i++) {
        m_mappedVertexArray->InsertNextTuple3(vertices[i][0], vertices[i][1], vertices[i][2]);
    }
    this->mapVertices();

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    m_lines = vtkSmartPointer<vtkLineSource>::New();
    m_lines->SetPoints(points);
    m_lines->Update();

    //Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    mapper->SetInputConnection(m_lines->GetOutputPort());

    m_actor->SetMapper(mapper);

    // Transform
    this->updateActorTransform();
}

void
LineMeshRenderDelegate::mapVertices()
{
    auto vertices = m_geometry->getVerticesPositionsNotConst();

    for (int i = 0; i < vertices.size(); i++) {
        m_mappedVertexArray->SetTuple3(i, vertices[i][0], vertices[i][1], vertices[i][2]);
    }

    // TODO: only when vertices modified
    m_mappedVertexArray->Modified();
}

void
LineMeshRenderDelegate::update()
{
    // Base class update
    RenderDelegate::update();

    this->mapVertices();
    m_lines->Update();
}

std::shared_ptr<Geometry>
LineMeshRenderDelegate::getGeometry() const
{
    return m_geometry;
}

} // imstk
