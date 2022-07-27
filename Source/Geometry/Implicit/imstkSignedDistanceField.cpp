/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSignedDistanceField.h"
#include "imstkDataArray.h"
#include "imstkImageData.h"
#include "imstkLogger.h"

namespace imstk
{
///
/// \brief Accepts structured coordinates (ie: pre int cast, [0, dim)) so it can do interpolation
/// origin should be image origin + spacing/2
///
template<typename T>
static T
trilinearSample(const Vec3d& structuredPt, T* imgPtr, const Vec3i& dim, const int numComps, const int comp)
{
    // minima of voxel, clamped to bounds
    const Vec3i s1 = structuredPt.cast<int>().cwiseMax(0).cwiseMin(dim - Vec3i(1, 1, 1));

    // maxima of voxel, clamped to bounds
    const Vec3i s2 = (structuredPt.cast<int>() + Vec3i(1, 1, 1)).cwiseMax(0).cwiseMin(dim - Vec3i(1, 1, 1));

    const size_t index000 = ImageData::getScalarIndex(s1.x(), s1.y(), s1.z(), dim, numComps) + comp;
    const size_t index100 = ImageData::getScalarIndex(s2.x(), s1.y(), s1.z(), dim, numComps) + comp;
    const size_t index110 = ImageData::getScalarIndex(s2.x(), s2.y(), s1.z(), dim, numComps) + comp;
    const size_t index010 = ImageData::getScalarIndex(s1.x(), s2.y(), s1.z(), dim, numComps) + comp;
    const size_t index001 = ImageData::getScalarIndex(s1.x(), s1.y(), s2.z(), dim, numComps) + comp;
    const size_t index101 = ImageData::getScalarIndex(s2.x(), s1.y(), s2.z(), dim, numComps) + comp;
    const size_t index111 = ImageData::getScalarIndex(s2.x(), s2.y(), s2.z(), dim, numComps) + comp;
    const size_t index011 = ImageData::getScalarIndex(s1.x(), s2.y(), s2.z(), dim, numComps) + comp;

    const double val000 = static_cast<double>(imgPtr[index000]);
    const double val100 = static_cast<double>(imgPtr[index100]);
    const double val110 = static_cast<double>(imgPtr[index110]);
    const double val010 = static_cast<double>(imgPtr[index010]);

    const double val001 = static_cast<double>(imgPtr[index001]);
    const double val101 = static_cast<double>(imgPtr[index101]);
    const double val111 = static_cast<double>(imgPtr[index111]);
    const double val011 = static_cast<double>(imgPtr[index011]);

    // Interpolants
    //const Vec3d t = s2.cast<double>() - structuredPt;
    const Vec3d t = structuredPt - s2.cast<double>();

    // Interpolate along x
    const double ax = val000 + (val100 - val000) * t[0];
    const double bx = val010 + (val110 - val010) * t[0];

    const double dx = val001 + (val101 - val001) * t[0];
    const double ex = val011 + (val111 - val011) * t[0];

    // Interpolate along y
    const double cy = ax + (bx - ax) * t[1];
    const double fy = dx + (ex - dx) * t[1];

    // Interpolate along z
    const double gz = cy + (fy - cy) * t[2];

    return static_cast<T>(gz);
}

SignedDistanceField::SignedDistanceField(std::shared_ptr<ImageData> imageData) :
    m_imageDataSdf(imageData), m_scale(1.0)
{
    m_invSpacing = m_imageDataSdf->getInvSpacing();
    m_bounds     = m_imageDataSdf->getBounds();
    m_shift      = m_imageDataSdf->getOrigin() - m_imageDataSdf->getSpacing() * 0.5;

    m_scalars = std::dynamic_pointer_cast<DataArray<double>>(m_imageDataSdf->getScalars());

    CHECK(m_scalars != nullptr)
        << "SignedDistanceField requires doubles in the input image";

    // \todo: Verify the SDF distances
}

double
SignedDistanceField::getFunctionValue(const Vec3d& pos) const
{
    // origin at center of first voxel
    if (pos[0] < m_bounds[1] && pos[0] > m_bounds[0]
        && pos[1] < m_bounds[3] && pos[1] > m_bounds[2]
        && pos[2] < m_bounds[5] && pos[2] > m_bounds[4])
    {
        const Vec3d structuredPt = (pos - m_shift).cwiseProduct(m_invSpacing);
        return trilinearSample(structuredPt, m_scalars->getPointer(), m_imageDataSdf->getDimensions(), 1, 0) * m_scale;
    }
    else
    {
        // If outside of the bounds, return positive (assume not inside)
        return IMSTK_DOUBLE_MAX;
    }
}

void
SignedDistanceField::computeBoundingBox(Vec3d& min, Vec3d& max, const double paddingPercent)
{
    return m_imageDataSdf->computeBoundingBox(min, max, paddingPercent);
}
} // namespace imstk