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

#include "imstkVTKFluidRenderDelegate.h"
#include "imstkPointSet.h"

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLFluidMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkTrivialProducer.h>
#include <vtkSphereSource.h>
#include <vtkDoubleArray.h>
#include <vtkGlyph3D.h>
#include <vtkVersion.h>
#include <vtkVolume.h>

namespace imstk
{
VTKFluidRenderDelegate::VTKFluidRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_visualModel = visualModel;

    auto geometry = std::static_pointer_cast<PointSet>(visualModel->getGeometry());

    // Map vertices
    StdVectorOfVec3d& vertices = geometry->getVertexPositionsNotConst();
    double*           vertData = reinterpret_cast<double*>(vertices.data());
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(vertData, vertices.size() * 3, 1);

    // Create points
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(geometry->getNumVertices());
    points->SetData(m_mappedVertexArray);

    // Create PolyData
    auto pointsPolydata = vtkSmartPointer<vtkPolyData>::New();
    pointsPolydata->SetPoints(points);

    // Create fluid mapper
    vtkNew<vtkOpenGLFluidMapper> fluidMapper;
    fluidMapper->SetInputData(pointsPolydata);

    // set some fluid rendering properties
    fluidMapper->SetParticleRadius(0.03f * 6.0f);
    fluidMapper->SetSurfaceFilterIterations(3);
    fluidMapper->SetSurfaceFilterRadius(5);
    fluidMapper->SetSurfaceFilterMethod(vtkOpenGLFluidMapper::FluidSurfaceFilterMethod::NarrowRange);
    fluidMapper->SetDisplayMode(vtkOpenGLFluidMapper::FluidDisplayMode::TransparentFluidVolume);
    fluidMapper->SetAttenuationColor(0.8f, 0.2f, 0.15f);
    fluidMapper->SetAttenuationScale(1.0f);
    fluidMapper->SetOpaqueColor(0.0f, 0.0f, 0.9f);
    fluidMapper->SetParticleColorPower(0.1f);
    fluidMapper->SetParticleColorScale(0.57f);
    fluidMapper->SetAdditionalReflection(0.0f);
    fluidMapper->SetRefractiveIndex(1.33f);
    fluidMapper->SetRefractionScale(0.07f);
    
    m_volume->SetMapper(fluidMapper);

    // Create connection source
    auto pointDataSource = vtkSmartPointer<vtkTrivialProducer>::New();
    pointDataSource->SetOutput(pointsPolydata);
    geometry->m_dataModified = true;

    // Update Transform, Render Properties
    this->update();
    this->setUpMapper(pointDataSource->GetOutputPort(), m_visualModel);

    m_isMesh = false;
    m_modelIsVolume = true;
}

void
VTKFluidRenderDelegate::updateDataSource()
{
    auto geometry = std::static_pointer_cast<PointSet>(m_visualModel->getGeometry());

    if (geometry->m_dataModified)
    {
        m_mappedVertexArray->Modified();
        geometry->m_dataModified = false;
    }
}
} // imstk
