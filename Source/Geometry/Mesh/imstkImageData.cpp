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
#include "imstkLogger.h"
#include "imstkDataArray.h"

namespace imstk
{
template<typename FROM_TYPE, typename TO_TYPE>
static void
castArray(std::shared_ptr<DataArray<FROM_TYPE>> fromArray, std::shared_ptr<DataArray<TO_TYPE>> toArray)
{
    const DataArray<FROM_TYPE>& fromArrayRef = *fromArray;
    DataArray<TO_TYPE>& toArrayRef = *toArray;
    const size_t                numVals = fromArray->size();
    for (size_t i = 0; i < numVals; i++)
    {
        toArrayRef[i] = static_cast<TO_TYPE>(fromArrayRef[i]);
    }
}

template<typename FROM_TYPE>
static void
castImage(std::shared_ptr<AbstractDataArray> fromScalars, std::shared_ptr<ImageData> toImage)
{
    const ScalarType toType = toImage->getScalars()->getScalarType();
    switch (toType)
    {
        TemplateMacro(castArray(
            std::dynamic_pointer_cast<DataArray<FROM_TYPE>>(fromScalars),
            std::dynamic_pointer_cast<DataArray<IMSTK_TT>>(toImage->getScalars())); );
    default:
        LOG(WARNING) << "Unknown scalar type";
        break;
    }
    ;
}

ImageData::ImageData(const std::string& name)
    : PointSet(Geometry::Type::ImageData, name),
    m_scalarArray(nullptr)
{
}

void
ImageData::print() const
{
    PointSet::print();
    LOG(INFO) << "Scalar Type: " << static_cast<int>(m_scalarArray->getScalarType());
    LOG(INFO) << "Number of Components" << m_numComps;
    LOG(INFO) << "Dimensions: " << m_dims[0] << ", " << m_dims[1] << ", " << m_dims[2];
    LOG(INFO) << "Spacing: " << m_spacing[0] << ", " << m_spacing[1] << ", " << m_spacing[2];
    LOG(INFO) << "Origin: " << m_origin[0] << ", " << m_origin[1] << ", " << m_origin[2];
    LOG(INFO) << "Bounds: ";
    LOG(INFO) << "\t" << m_bounds[0] << ", " << m_bounds[1];
    LOG(INFO) << "\t" << m_bounds[2] << ", " << m_bounds[3];
    LOG(INFO) << "\t" << m_bounds[4] << ", " << m_bounds[5];
}

double
ImageData::getVolume() const
{
    if (m_scalarArray == nullptr)
    {
        return 0.0;
    }

    return (m_dims[0] * m_spacing.x()) *
           (m_dims[1] * m_spacing.y()) *
           (m_dims[2] * m_spacing.z());
}

void*
ImageData::getVoidPointer()
{
    return m_scalarArray->getVoidPointer();
}

std::shared_ptr<ImageData>
ImageData::cast(ScalarType toType)
{
    // Create image of new type
    std::shared_ptr<ImageData> results = std::make_shared<ImageData>();
    results->allocate(toType, m_numComps, m_dims, m_spacing, m_origin);
    switch (m_scalarArray->getScalarType())
    {
        TemplateMacro(castImage<IMSTK_TT>(m_scalarArray, results); );
    default:
        LOG(WARNING) << "Unknown scalar type";
        break;
    }
    ;
    return results;
}

const ScalarType
ImageData::getScalarType() const
{
    return m_scalarArray->getScalarType();
}

void
ImageData::setScalars(std::shared_ptr<AbstractDataArray> scalars, const int numComps, int* dim)
{
    CHECK(scalars != nullptr);
    if (dim[0] * dim[1] * dim[2] * numComps != scalars->size())
    {
        LOG(WARNING) << "Scalars don't align";
    }
    m_scalarArray = scalars;
    m_dims[0]     = dim[0];
    m_dims[1]     = dim[1];
    m_dims[2]     = dim[2];
    m_numComps    = numComps;
}

void
ImageData::allocate(const ScalarType type, const int numComps, const Vec3i& dims, const Vec3d& spacing, const Vec3d& origin)
{
    m_dims   = dims;
    m_origin = origin;
    setSpacing(spacing);
    m_numComps = numComps;
    const size_t numVals = static_cast<size_t>(dims[0] * dims[1] * dims[2] * numComps);
    switch (type)
    {
        TemplateMacro(m_scalarArray = std::make_shared<DataArray<IMSTK_TT>>(numVals););
    default:
        LOG(WARNING) << "Tried to allocate unknown scalar type";
        break;
    }
    // When allocation is done, post modified on the image
    connect<Event>(m_scalarArray, EventType::Modified, [&](Event*) { this->emit(Event(EventType::Modified)); });
}

void
ImageData::computePoints()
{
    StdVectorOfVec3d vertices(m_dims[0] * m_dims[1] * m_dims[2]);
    const Vec3d      shift = m_origin + m_spacing * 0.5;
    int              i     = 0;
    for (int z = 0; z < m_dims[2]; z++)
    {
        for (int y = 0; y < m_dims[1]; y++)
        {
            for (int x = 0; x < m_dims[0]; x++, i++)
            {
                vertices[i] = Vec3d(x, y, z).cwiseProduct(m_spacing) + shift;
            }
        }
    }
    setInitialVertexPositions(vertices);
    setVertexPositions(vertices);
}

void
ImageData::clear()
{
    if (this->m_scalarArray != nullptr)
    {
        this->m_scalarArray = nullptr;
    }
    //this->m_dataTransform->Identity();
    this->m_transformApplied = true;
    this->m_dataModified     = true;
}
} // imstk
