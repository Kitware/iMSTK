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
VTKdbgTrianglesRenderDelegate::VTKdbgTrianglesRenderDelegate(const std::shared_ptr<DebugRenderTriangles>& renderTriangles) :
    m_RenderGeoData(renderTriangles),
    m_pappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    // Map vertices in memory
    m_pappedVertexArray->SetNumberOfComponents(3);

    // Create points
    m_points = vtkSmartPointer<vtkPoints>::New();
    m_points->SetData(m_pappedVertexArray);

    // Create cells
    m_cellArray = vtkSmartPointer<vtkCellArray>::New();

    // Create PolyData
    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(m_points);
    m_polyData->SetPolys(m_cellArray);

    // Create connection source
    auto source = vtkSmartPointer<vtkTrivialProducer>::New();
    source->SetOutput(m_polyData);

    // Update Transform, Render Properties
    //updateDataSource();
    //updateActorProperties();

    setUpMapper(source->GetOutputPort(), true, m_RenderGeoData->getRenderMaterial());
}

void
VTKdbgTrianglesRenderDelegate::updateDataSource()
{
    if (m_RenderGeoData->isModified())
    {
        m_RenderGeoData->setDataModified(false);
        m_pappedVertexArray->SetArray(m_RenderGeoData->getVertexBufferPtr(),
                                      m_RenderGeoData->getNumVertices() * 3, 1);

        // Set point data
        m_points->SetNumberOfPoints(m_RenderGeoData->getNumVertices());

        // Set tri data
        int numCurrentTriangles = m_cellArray->GetNumberOfCells();
        if (numCurrentTriangles > static_cast<int>(m_RenderGeoData->getNumVertices() / 3))
        {
            // There should is a better way to modify the existing data,
            // instead of discarding everything and add from the beginning
            m_cellArray->Reset();
            numCurrentTriangles = 0;
        }

        vtkIdType cell[3];
        for (int i = numCurrentTriangles; i < static_cast<int>(m_RenderGeoData->getNumVertices() / 3); ++i)
        {
            cell[0] = 3 * i;
            cell[1] = cell[0] + 1;
            cell[2] = cell[0] + 2;
            m_cellArray->InsertNextCell(3, cell);
        }

        m_pappedVertexArray->Modified();

        // Sleep for a while, wating for the data to propagate.
        // This is necessary to avoid access violation error during CPU/GPU data transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
} // imstk
