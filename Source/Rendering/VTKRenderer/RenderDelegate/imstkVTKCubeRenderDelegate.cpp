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

#include "imstkVTKCubeRenderDelegate.h"

#include "vtkCubeSource.h"

namespace imstk
{
VTKCubeRenderDelegate::VTKCubeRenderDelegate(std::shared_ptr<Cube> cube) :
    m_geometry(cube),
    m_transformFilter(vtkSmartPointer<vtkTransformPolyDataFilter>::New())
{
    auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource->SetCenter(0, 0, 0);
    cubeSource->SetXLength(1.0);
    cubeSource->SetYLength(1.0);
    cubeSource->SetZLength(1.0);

    m_transformFilter->SetInputConnection(cubeSource->GetOutputPort());
    m_transformFilter->SetTransform(vtkSmartPointer<vtkTransform>::New());

    this->update();
    this->setUpMapper(m_transformFilter->GetOutputPort(), true, m_geometry->getRenderMaterial());
}

void
VTKCubeRenderDelegate::updateDataSource()
{
    if (!m_geometry->m_dataModified)
    {
        return;
    }

    Geometry::DataType type = Geometry::DataType::PreTransform;

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(m_geometry->getPosition(type));
    T.rotate(Quatd::FromTwoVectors(UP_VECTOR, m_geometry->getOrientationAxis(type)));
    T.scale(m_geometry->getWidth(type));
    T.matrix().transposeInPlace();

    auto vtkT = vtkTransform::SafeDownCast(m_transformFilter->GetTransform());
    vtkT->SetMatrix(T.data());

    m_geometry->m_dataModified = false;
}

std::shared_ptr<Geometry>
VTKCubeRenderDelegate::getGeometry() const
{
    return m_geometry;
}
} // imstk
