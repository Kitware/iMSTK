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

#include "imstkCube.h"

namespace imstk
{
void
Cube::print() const
{
    Geometry::print();
    LOG(INFO) << "Width: " << m_width;
}

double
Cube::getVolume() const
{
    return m_width*m_width*m_width;
}

double
Cube::getWidth(DataType type /* = DataType::PostTransform */)
{
    if (type == DataType::PostTransform)
    {
        this->updatePostTransformData();
        return m_widthPostTransform;
    }
    return m_width;
}

void
Cube::setWidth(const double w)
{
    if (w <= 0)
    {
        LOG(WARNING) << "Cube::setWidth error: width should be positive.";
        return;
    }
    if (m_width == w)
    {
        return;
    }
    m_width = w;
    m_dataModified = true;
    m_transformApplied = false;
}

void
Cube::applyScaling(const double s)
{
    this->setWidth(m_width * s);
}

void
Cube::updatePostTransformData()
{
    if (m_transformApplied)
    {
        return;
    }
    AnalyticalGeometry::updatePostTransformData();
    m_widthPostTransform = m_scaling * m_width;
    m_transformApplied = true;
}
} // imstk
