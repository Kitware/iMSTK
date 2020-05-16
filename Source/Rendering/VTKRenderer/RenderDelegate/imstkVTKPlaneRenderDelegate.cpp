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

#include <vtkTransformPolyDataFilter.h>
#include <vtkPlaneSource.h>

namespace imstk
{
VTKPlaneRenderDelegate::VTKPlaneRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_transformFilter(vtkSmartPointer<vtkTransformPolyDataFilter>::New())
{
    m_visualModel = visualModel;

    //auto geometry = std::static_pointer_cast<Plane>(m_visualModel->getGeometry());

    auto planeSource = vtkSmartPointer<vtkPlaneSource>::New();
    planeSource->SetCenter(0, 0, 0);
    planeSource->SetNormal(0, 1, 0);

    m_transformFilter->SetInputConnection(planeSource->GetOutputPort());
    m_transformFilter->SetTransform(vtkSmartPointer<vtkTransform>::New());

    this->update();
    this->setUpMapper(m_transformFilter->GetOutputPort(), m_visualModel);
}

void
VTKPlaneRenderDelegate::updateDataSource()
{
    auto geometry = std::static_pointer_cast<Plane>(m_visualModel->getGeometry());

    if (!geometry->m_dataModified)
    {
        return;
    }

    Geometry::DataType type = Geometry::DataType::PreTransform;

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(type));
    T.rotate(Quatd::FromTwoVectors(UP_VECTOR, geometry->getNormal(type)));
    T.scale(geometry->getWidth(type));
    T.matrix().transposeInPlace();

    auto vtkT = vtkTransform::SafeDownCast(m_transformFilter->GetTransform());
    vtkT->SetMatrix(T.data());

    geometry->m_dataModified = false;
}
} // imstk
