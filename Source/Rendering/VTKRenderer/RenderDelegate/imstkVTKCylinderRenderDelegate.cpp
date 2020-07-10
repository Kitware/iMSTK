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

#include "imstkVTKCylinderRenderDelegate.h"
#include "imstkCylinder.h"
#include "imstkVisualModel.h"

#include <vtkCylinderSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

namespace imstk
{
VTKCylinderRenderDelegate::VTKCylinderRenderDelegate(std::shared_ptr<VisualModel> visualModel) :
    m_transformFilter(vtkSmartPointer<vtkTransformPolyDataFilter>::New())
{
    m_visualModel = visualModel;

    auto geometry = std::static_pointer_cast<Cylinder>(visualModel->getGeometry());

    //Geometry::DataType type = Geometry::DataType::PreTransform;
    cylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
    cylinderSource->SetCenter(0., 0., 0.);
    cylinderSource->SetRadius(1.);
    cylinderSource->SetHeight(geometry->getLength());
    cylinderSource->SetResolution(100);

    m_transformFilter->SetInputConnection(cylinderSource->GetOutputPort());
    m_transformFilter->SetTransform(vtkSmartPointer<vtkTransform>::New());

    this->update();
    this->setUpMapper(m_transformFilter->GetOutputPort(), m_visualModel);
}

void
VTKCylinderRenderDelegate::updateDataSource()
{
    auto geometry = std::static_pointer_cast<Cylinder>(m_visualModel->getGeometry());

    if (!geometry->m_dataModified)
    {
        return;
    }

    Geometry::DataType type = Geometry::DataType::PreTransform;

    cylinderSource->SetRadius(geometry->getRadius());
    cylinderSource->SetHeight(geometry->getLength());

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(type));
    T.rotate(Quatd::FromTwoVectors(UP_VECTOR, geometry->getOrientationAxis(type)));
    T.scale(1.0);
    T.matrix().transposeInPlace();

    auto vtkT = vtkTransform::SafeDownCast(m_transformFilter->GetTransform());
    vtkT->SetMatrix(T.data());

    geometry->m_dataModified = false;
}
} // imstk
