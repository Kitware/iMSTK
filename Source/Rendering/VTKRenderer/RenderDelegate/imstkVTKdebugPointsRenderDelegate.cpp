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

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkTrivialProducer.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkProperty.h>
#include <vtkGlyph3D.h>
#include <vtkPoints.h>

namespace imstk
{
VTKdbgPointsRenderDelegate::VTKdbgPointsRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_visualModel = visualModel;

    // Map vertices
    m_mappedVertexArray->SetNumberOfComponents(3);

    // Create points
    m_points = vtkSmartPointer<vtkPoints>::New();
    m_points->SetData(m_mappedVertexArray);

    // Create PolyData
    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(m_points);

    auto sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_glyph = vtkSmartPointer<vtkGlyph3D>::New();
    m_glyph->SetSourceConnection(sphereSource->GetOutputPort());
    m_glyph->SetInputData(m_polyData);

    // Update Transform, Render Properties
    updateActorProperties();
    setUpMapper(m_glyph->GetOutputPort(), false, visualModel->getRenderMaterial());

    //updateDataSource();
}

void
VTKdbgPointsRenderDelegate::updateDataSource()
{
    auto dbgPoints = std::static_pointer_cast<DebugRenderPoints>(m_visualModel->getDebugGeometry());

    if (dbgPoints->isModified())
    {
        dbgPoints->setDataModified(false);
        m_mappedVertexArray->SetArray(dbgPoints->getVertexBufferPtr(),
                                      dbgPoints->getNumVertices() * 3, 1);

        // Update points geometry
        // m_Points need to be created from scrach, otherwise program will crash
        m_points = vtkSmartPointer<vtkPoints>::New();
        m_points->SetNumberOfPoints(dbgPoints->getNumVertices());
        m_points->SetData(m_mappedVertexArray);
        m_polyData->SetPoints(m_points);

        m_mappedVertexArray->Modified();

        // Sleep for a while, wating for the data to propagate
        // This is necessary to avoid access violation error during CPU/GPU data transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
} // imstk
