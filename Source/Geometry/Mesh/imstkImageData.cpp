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

#include "imstkImageData.h"

// vtk
#include "vtkImageReslice.h"
#include "vtkTransform.h"

namespace imstk
{
ImageData::ImageData(const std::string name)
    : PointSet(Geometry::Type::ImageData, name),
    m_dataTransform(vtkSmartPointer<vtkTransform>::New())
{
}

void
ImageData::print() const
{
    Geometry::print();
}

double
ImageData::getVolume() const
{
    if (!this->m_data)
    {
        return 0.0;
    }

    double bounds[6];
    this->m_data->GetBounds(bounds);
    return ((bounds[1] - bounds[0]) *
            (bounds[3] - bounds[2]) *
            (bounds[5] - bounds[4]));
}

void
ImageData::initialize(vtkImageData* im)
{
    this->clear();
    if (im)
    {
        this->m_data = vtkSmartPointer<vtkImageData>::New();
        this->m_data->DeepCopy(im);
    }
}

void
ImageData::clear()
{
    if (this->m_data)
    {
        this->m_data = nullptr;
    }
    this->m_dataTransform->Identity();
    this->m_transformApplied = true;
    this->m_dataModified     = true;
}

vtkImageData*
ImageData::getData(DataType type)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
    }
    return this->m_data;
}

void
ImageData::applyTranslation(const Vec3d t)
{
    this->m_dataTransform->Translate(t[0], t[1], t[2]);

    this->m_dataModified     = true;
    this->m_transformApplied = false;
}

void
ImageData::applyScaling(const double s)
{
    this->m_dataTransform->Scale(s, s, s);

    this->m_dataModified     = true;
    this->m_transformApplied = false;
}

void
ImageData::applyRotation(const Mat3d r)
{
    vtkNew<vtkMatrix4x4> mat;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            mat->SetElement(i, j, r(i, j));
        }
    }
    this->m_dataTransform->Concatenate(mat);

    this->m_dataModified     = true;
    this->m_transformApplied = false;
}

void
ImageData::updatePostTransformData() const
{
    if (m_transformApplied || !this->m_data)
    {
        return;
    }

    vtkNew<vtkImageReslice> reslice;
    reslice->SetInputData(this->m_data);
    reslice->SetResliceTransform(this->m_dataTransform);
    reslice->SetInterpolationModeToLinear();
    reslice->Update();
    this->m_data->DeepCopy(reslice->GetOutput());
    this->m_transformApplied = true;
}
} // imstk
