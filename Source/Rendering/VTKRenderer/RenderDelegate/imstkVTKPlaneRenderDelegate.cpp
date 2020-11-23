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

#include "imstkVTKPlaneRenderDelegate.h"
#include "imstkPlane.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>

namespace imstk
{
VTKPlaneRenderDelegate::VTKPlaneRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel)
{
    //auto geometry = std::static_pointer_cast<Plane>(m_visualModel->getGeometry());

    vtkNew<vtkPlaneSource> planeSource;
    planeSource->SetCenter(0.0, 0.0, 0.0);
    planeSource->SetNormal(0.0, 1.0, 0.0);

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(planeSource->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = actor;
    }

    update();
    updateRenderProperties();
}

void
VTKPlaneRenderDelegate::processEvents()
{
    VTKRenderDelegate::processEvents();

    auto geometry = std::static_pointer_cast<Plane>(m_visualModel->getGeometry());

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(Geometry::DataType::PostTransform));
    T.rotate(Quatd::FromTwoVectors(UP_VECTOR, geometry->getNormal(Geometry::DataType::PostTransform)));
    T.scale(geometry->getWidth(Geometry::DataType::PostTransform));
    T.matrix().transposeInPlace();

    m_transform->SetMatrix(T.data());
}
} // imstk
