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

namespace imstk
{
VTKCylinderRenderDelegate::VTKCylinderRenderDelegate(std::shared_ptr<Cylinder> cylinder) :
    m_geometry(cylinder),
    m_transformFilter(vtkSmartPointer<vtkTransformPolyDataFilter>::New())
{
    Geometry::DataType type = Geometry::DataType::PreTransform;
    cylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
    cylinderSource->SetCenter(0., 0., 0.);
    cylinderSource->SetRadius(1.);
    cylinderSource->SetHeight(m_geometry->getLength());
    cylinderSource->SetResolution(100);

    m_transformFilter->SetInputConnection(cylinderSource->GetOutputPort());
    m_transformFilter->SetTransform(vtkSmartPointer<vtkTransform>::New());

    this->update();
    this->setUpMapper(m_transformFilter->GetOutputPort(), true, m_geometry->getRenderMaterial());
}

void
VTKCylinderRenderDelegate::updateDataSource()
{
    if (!m_geometry->m_dataModified)
    {
        return;
    }

    Geometry::DataType type = Geometry::DataType::PreTransform;

    cylinderSource->SetRadius(m_geometry->getRadius());
    cylinderSource->SetHeight(m_geometry->getLength());

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(m_geometry->getPosition(type));
    T.rotate(Quatd::FromTwoVectors(UP_VECTOR, m_geometry->getOrientationAxis(type)));
    T.scale(1.0);
    T.matrix().transposeInPlace();

    auto vtkT = vtkTransform::SafeDownCast(m_transformFilter->GetTransform());
    vtkT->SetMatrix(T.data());

    m_geometry->m_dataModified = false;
}

std::shared_ptr<Geometry>
VTKCylinderRenderDelegate::getGeometry() const
{
    return m_geometry;
}
} // imstk
