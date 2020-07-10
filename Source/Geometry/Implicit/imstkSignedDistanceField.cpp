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

#include "imstkSignedDistanceField.h"
#include "imstkDataArray.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
///
/// \brief Accepts structured coordinates (ie: pre int cast, [0, dim)) so it can do interpolation
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
    const Vec3d t = s2.cast<double>() - structuredPt;

    // Interpolate along x
    const double ax = val000 + (val000 - val100) * t[0];
    const double bx = val010 + (val010 - val110) * t[0];

    const double dx = val001 + (val001 - val101) * t[0];
    const double ex = val011 + (val011 - val111) * t[0];

    // Interpolate along y
    const double cy = ax + (ax - bx) * t[1];
    const double fy = dx + (dx - ex) * t[1];

    // Interpolate along z
    const double gz = fy + (cy - fy) * t[2];

    return static_cast<T>(gz);
}

///
/// \brief Trilinearly samples out the gradient, could precompute
///
template<typename T>
static Vec3d
trilinearGrad(const Vec3d& pt, T* imgPtr, const Vec3i& dim)
{
    const double xminVal = static_cast<double>(trilinearSample(Vec3d(pt[0] - 1.0, pt[1], pt[2]), imgPtr, dim, 1, 0));
    const double xmaxVal = static_cast<double>(trilinearSample(Vec3d(pt[0] + 1.0, pt[1], pt[2]), imgPtr, dim, 1, 0));
    const double yminVal = static_cast<double>(trilinearSample(Vec3d(pt[0], pt[1] - 1.0, pt[2]), imgPtr, dim, 1, 0));
    const double ymaxVal = static_cast<double>(trilinearSample(Vec3d(pt[0], pt[1] + 1.0, pt[2]), imgPtr, dim, 1, 0));
    const double zminVal = static_cast<double>(trilinearSample(Vec3d(pt[0], pt[1], pt[2] - 1.0), imgPtr, dim, 1, 0));
    const double zmaxVal = static_cast<double>(trilinearSample(Vec3d(pt[0], pt[1], pt[2] + 1.0), imgPtr, dim, 1, 0));

    return Vec3d(xmaxVal - xminVal, ymaxVal - yminVal, zmaxVal - zminVal);
}

SignedDistanceField::SignedDistanceField(std::shared_ptr<ImageData> imageData, std::string name) :
    ImplicitGeometry(Type::SDF, name),
    m_imageDataSdf(imageData)
{
    const Vec3d& spacing = m_imageDataSdf->getSpacing();
    invSpacing = Vec3d(1.0 / spacing[0], 1.0 / spacing[1], 1.0 / spacing[2]);
    bounds     = m_imageDataSdf->getBounds();

    if (m_imageDataSdf->getScalarType() != IMSTK_FLOAT)
    {
        LOG(WARNING) << "SignedDistanceField requires float input image";
        return;
    }

    m_scalars = std::dynamic_pointer_cast<DataArray<float>>(m_imageDataSdf->getScalars());
}

double
SignedDistanceField::getFunctionValue(const Vec3d& pos) const
{
    if (pos[0] < bounds[0] || pos[0] > bounds[1] || pos[1] < bounds[2] || pos[1] > bounds[3] || pos[2] < bounds[4] || pos[2] > bounds[5])
    {
        return IMSTK_DOUBLE_MAX;
    }
    else
    {
        const Vec3d structuredPt = (pos - m_imageDataSdf->getOrigin()).cwiseProduct(invSpacing);
        return trilinearSample(structuredPt, m_scalars->getPointer(), m_imageDataSdf->getDimensions(), 1, 0);
    }
}

Vec3d
SignedDistanceField::getFunctionGrad(const Vec3d& pos) const
{
    if (pos[0] < bounds[0] || pos[0] > bounds[1] || pos[1] < bounds[2] || pos[1] > bounds[3] || pos[2] < bounds[4] || pos[2] > bounds[5])
    {
        return Vec3d(0.0, 0.0, 0.0);
    }
    else
    {
        const Vec3d structuredPt = (pos - m_imageDataSdf->getOrigin()).cwiseProduct(invSpacing);
        return trilinearGrad(structuredPt, m_scalars->getPointer(), m_imageDataSdf->getDimensions());
    }
}
}