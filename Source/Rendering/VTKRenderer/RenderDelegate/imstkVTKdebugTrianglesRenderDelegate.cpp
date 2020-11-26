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
#include "imstkDebugRenderGeometry.h"
#include "imstkVisualModel.h"

#include <thread>
#include <vtkActor.h>
#include <vtkDoubleArray.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkPolyData.h>

namespace imstk
{
VTKdbgTrianglesRenderDelegate::VTKdbgTrianglesRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel),
    m_paddedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    // Map vertices in memory
    m_paddedVertexArray->SetNumberOfComponents(3);

    // Create points
    m_points = vtkSmartPointer<vtkPoints>::New();
    m_points->SetData(m_paddedVertexArray);

    // Create cells
    m_cellArray = vtkSmartPointer<vtkCellArray>::New();

    // Create PolyData
    m_polyData = vtkSmartPointer<vtkPolyData>::New();
    m_polyData->SetPoints(m_points);
    m_polyData->SetPolys(m_cellArray);

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(m_polyData);
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        //actor->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = actor;
        if (auto mapper = vtkOpenGLPolyDataMapper::SafeDownCast(m_mapper.GetPointer()))
        {
            mapper->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);
        }
    }

    updateRenderProperties();
}

void
VTKdbgTrianglesRenderDelegate::processEvents()
{
    auto dbgTriangles = std::static_pointer_cast<DebugRenderTriangles>(m_visualModel->getDebugGeometry());

    if (dbgTriangles->isModified())
    {
        dbgTriangles->setDataModified(false);
        m_paddedVertexArray->SetArray(dbgTriangles->getVertexBufferPtr(),
                                      dbgTriangles->getNumVertices() * 3, 1);

        // Set point data
        m_points->SetNumberOfPoints(dbgTriangles->getNumVertices());

        // Set tri data
        int numCurrentTriangles = m_cellArray->GetNumberOfCells();
        if (numCurrentTriangles > static_cast<int>(dbgTriangles->getNumVertices() / 3))
        {
            // There should is a better way to modify the existing data,
            // instead of discarding everything and add from the beginning
            m_cellArray->Reset();
            numCurrentTriangles = 0;
        }

        vtkIdType cell[3];
        for (int i = numCurrentTriangles; i < static_cast<int>(dbgTriangles->getNumVertices() / 3); ++i)
        {
            cell[0] = 3 * i;
            cell[1] = cell[0] + 1;
            cell[2] = cell[0] + 2;
            m_cellArray->InsertNextCell(3, cell);
        }

        m_paddedVertexArray->Modified();

        // Sleep for a while, wating for the data to propagate.
        // This is necessary to avoid access violation error during CPU/GPU data transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
} // imstk
