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
#include <vtkProperty.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkTrivialProducer.h>
#include <vtkLine.h>

namespace imstk
{
VTKdbgLinesRenderDelegate::VTKdbgLinesRenderDelegate(const std::shared_ptr<DebugRenderLines>& lineData) :
    m_RenderGeoData(lineData),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    // Map vertices
    m_mappedVertexArray->SetNumberOfComponents(3);

    // Create points
    m_points = vtkSmartPointer<vtkPoints>::New();
    m_points->SetData(m_mappedVertexArray);

    // Create cells
    m_cellArray = vtkSmartPointer<vtkCellArray>::New();

    // Create lines polydata
    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(m_points);
    m_polyData->SetLines(m_cellArray);

    // Create connection source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(m_polyData);

    // Update Transform, Render Properties
    updateActorProperties();
    setUpMapper(source->GetOutputPort(), false, m_RenderGeoData->getRenderMaterial());

    //updateDataSource();
}

void
VTKdbgLinesRenderDelegate::updateDataSource()
{
    if (m_RenderGeoData->isModified())
    {
        m_RenderGeoData->setDataModified(false);
        m_mappedVertexArray->SetArray(m_RenderGeoData->getVertexBufferPtr(),
                                      m_RenderGeoData->getNumVertices() * 3, 1);

        m_points->SetNumberOfPoints(m_RenderGeoData->getNumVertices());

        // Set line data
        int numCurrentLines = m_cellArray->GetNumberOfCells();
        if (numCurrentLines > static_cast<int>(m_RenderGeoData->getNumVertices() / 2))
        {
            // There should is a better way to modify the existing data,
            // instead of discarding everything and add from the beginning
            m_cellArray->Reset();
            numCurrentLines = 0;
        }

        vtkIdType cell[2];
        for (int i = numCurrentLines; i < static_cast<int>(m_RenderGeoData->getNumVertices() / 2); ++i)
        {
            cell[0] = 2 * i;
            cell[1] = cell[0] + 1;
            m_cellArray->InsertNextCell(2, cell);
        }

        m_mappedVertexArray->Modified();

        // Sleep for a while, wating for the data to propagate.
        // This is necessary to avoid access violation error during CPU/GPU data transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
} // imstk
