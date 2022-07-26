/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkDataArray.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkLooseOctree.h"
#include "imstkParallelUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkTimer.h"
#include "imstkSurfaceMeshImageMask.h"
#include <stack>
#include <vtkDistancePolyDataFilter.h>
#include <vtkImageData.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkOctreePointLocator.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>

namespace imstk
{
///
/// \brief Only works with binary image
/// returns 0 if neighborhood is equivalent to val
/// returns 1 if neighborhood is filled with 1's
/// returns -1 if neighborhood is filled with 0's
///
//static int isNeighborhoodEquivalent(const Vec3i& pt, const Vec3i& dim, const double val, const double* imgPtr, const int dilateSize)
//{
//    const Vec3i min = (pt - Vec3i(dilateSize, dilateSize, dilateSize)).cwiseMax(Vec3i(0, 0, 0)).cwiseMin(dim - Vec3i(1, 1, 1));
//    const Vec3i max = (pt + Vec3i(dilateSize, dilateSize, dilateSize)).cwiseMax(Vec3i(0, 0, 0)).cwiseMin(dim - Vec3i(1, 1, 1));
//
//    // Take the max of the neighborhood
//    for (int z = min[2]; z < max[2] + 1; z++)
//    {
//        for (int y = min[1]; y < max[1] + 1; y++)
//        {
//            for (int x = min[0]; x < max[0] + 1; x++)
//            {
//                const int index = ImageData::getScalarIndex(x, y, z, dim, 1);
//                if (val != imgPtr[index])
//                {
//                    return false;
//                }
//            }
//        }
//    }
//    return 0;
//}
static bool
isNeighborhoodEquivalent(const Vec3i& pt, const Vec3i& dim, const float val, const float* imgPtr, const int dilateSize)
{
    const Vec3i min = (pt - Vec3i(dilateSize, dilateSize, dilateSize)).cwiseMax(Vec3i(0, 0, 0)).cwiseMin(dim - Vec3i(1, 1, 1));
    const Vec3i max = (pt + Vec3i(dilateSize, dilateSize, dilateSize)).cwiseMax(Vec3i(0, 0, 0)).cwiseMin(dim - Vec3i(1, 1, 1));

    // Take the max of the neighborhood
    for (int z = min[2]; z < max[2] + 1; z++)
    {
        for (int y = min[1]; y < max[1] + 1; y++)
        {
            for (int x = min[0]; x < max[0] + 1; x++)
            {
                const size_t index = ImageData::getScalarIndex(x, y, z, dim, 1);
                if (val != imgPtr[index])
                {
                    return false;
                }
            }
        }
    }
    return true;
}

// Narrow band is WIP, it works but is slow
static void
computeNarrowBandedDT(std::shared_ptr<ImageData> imageData, std::shared_ptr<SurfaceMesh> surfMesh, const int dilateSize)
{
    // Rasterize a mask from the polygon
    std::shared_ptr<SurfaceMeshImageMask> imageMask = std::make_shared<SurfaceMeshImageMask>();
    imageMask->setInputMesh(surfMesh);
    imageMask->setReferenceImage(imageData);
    imageMask->update();

    DataArray<float>&  inputScalars  = *std::dynamic_pointer_cast<DataArray<float>>(imageMask->getOutputImage()->getScalars());
    float*             inputImgPtr   = inputScalars.getPointer();
    DataArray<double>& outputScalars = *std::dynamic_pointer_cast<DataArray<double>>(imageData->getScalars());
    double*            outputImgPtr  = outputScalars.getPointer();

    vtkSmartPointer<vtkPolyData>                 inputPolyData = GeometryUtils::copyToVtkPolyData(surfMesh);
    vtkSmartPointer<vtkImplicitPolyDataDistance> polyDataDist  = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    polyDataDist->SetInput(inputPolyData);
    std::fill_n(outputImgPtr, outputScalars.size(), 10000.0);

    // Iterate the image testing for boundary pixels (ie any 0 adjacent to a 1)
    const Vec3i& dim     = imageData->getDimensions();
    const Vec3d  shift   = imageData->getOrigin() + imageData->getSpacing() * 0.5;
    const Vec3d& spacing = imageData->getSpacing();
    int          i       = 0;
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++, i++)
            {
                const float val = inputImgPtr[i];
                const Vec3i pt  = Vec3i(x, y, z);

                // If neighborhood is homogenous then its not touching the boundary
                if (!isNeighborhoodEquivalent(pt, dim, val, inputImgPtr, dilateSize))
                {
                    const Vec3d pos = pt.cast<double>().cwiseProduct(spacing) + shift;
                    outputImgPtr[i] = polyDataDist->FunctionValue(pos.data());
                }
                // If value is 1 (we are inside)
                else if (val == 1.0)
                {
                    outputImgPtr[i] = -10000.0;
                }

                if (i % 1000000 == 0)
                {
                    double p = static_cast<double>(i) / (dim[0] * dim[1] * dim[2]);
                    std::cout << "Progress " << p << "\n";
                }
            }
        }
    }
}

static void
computeFullDT(std::shared_ptr<ImageData> imageData, std::shared_ptr<SurfaceMesh> surfMesh, double tolerance)
{
    // Get the optimal number of threads
    const size_t numThreads = ParallelUtils::ThreadManager::getThreadPoolSize();

    const Vec3i& dim     = imageData->getDimensions();
    const Vec3d  spacing = imageData->getSpacing();
    const Vec3d  shift   = imageData->getOrigin() + spacing * 0.5;

    DataArray<double>& scalars = *static_cast<DataArray<double>*>(imageData->getScalars().get());

    // Split the work up along z using thread count to avoid making too many octtrees (may not be most optimal)
    ParallelUtils::parallelFor(numThreads, [&](const size_t& i)
        {
            // Separate polygons used to avoid race conditions
            vtkSmartPointer<vtkPolyData> inputPolyData = GeometryUtils::copyToVtkPolyData(surfMesh);
            vtkSmartPointer<vtkImplicitPolyDataDistance> polyDataDist = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
            polyDataDist->SetInput(inputPolyData);
            polyDataDist->SetTolerance(tolerance);
            for (int z = static_cast<int>(i); z < dim[2]; z += static_cast<int>(numThreads))
            {
                int j = z * dim[0] * dim[1];
                for (int y = 0; y < dim[1]; y++)
                {
                    for (int x = 0; x < dim[0]; x++, j++)
                    {
                        double pos[3] = { x* spacing[0] + shift[0], y * spacing[1] + shift[1], z * spacing[2] + shift[2] };
                        scalars[j]    = polyDataDist->FunctionValue(pos);
                    }
                }
            }
        });

    // Sequential implementation
    /*vtkSmartPointer<vtkPolyData> inputPolyData = GeometryUtils::copyToVtkPolyData(surfMesh);
    vtkSmartPointer<vtkImplicitPolyDataDistance> polyDataDist = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    polyDataDist->SetInput(inputPolyData);
    int j = 0;
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++, j++)
            {
                double pos[3] = { x * spacing[0] + shift[0], y * spacing[1] + shift[1], z * spacing[2] + shift[2] };
                scalars[j] = polyDataDist->FunctionValue(pos);
            }
        }
    }*/
}

SurfaceMeshDistanceTransform::SurfaceMeshDistanceTransform()
{
    setNumInputPorts(1);
    setRequiredInputType<SurfaceMesh>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<ImageData>(), 0);
}

void
SurfaceMeshDistanceTransform::setInputMesh(std::shared_ptr<SurfaceMesh> mesh)
{
    setInput(mesh, 0);
}

std::shared_ptr<ImageData>
SurfaceMeshDistanceTransform::getOutputImage()
{
    return std::dynamic_pointer_cast<ImageData>(getOutput());
}

void
SurfaceMeshDistanceTransform::setBounds(const Vec6d& bounds)
{
    m_Bounds = bounds;
    if (m_Bounds.isZero())
    {
        LOG(WARNING) << "SurfaceMeshDistanceTransform Bounds are zero, the input SurfaceMesh bounds will be used instead.";
    }
}

void
SurfaceMeshDistanceTransform::setBounds(const Vec3d& min, const Vec3d& max)
{
    m_Bounds << min.x(), max.x(), min.y(), max.y(), min.z(), max.z();
    if (m_Bounds.isZero())
    {
        LOG(WARNING) << "SurfaceMeshDistanceTransform Bounds are zero, the input SurfaceMesh bounds will be used instead.";
    }
}

void
SurfaceMeshDistanceTransform::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> inputSurfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    std::shared_ptr<ImageData>   outputImageData  = std::dynamic_pointer_cast<ImageData>(getOutput(0));

    if (m_Dimensions[0] == 0 || m_Dimensions[1] == 0 || m_Dimensions[2] == 0)
    {
        LOG(WARNING) << "SurfaceMeshDistanceTransform Dimensions not set";
        return;
    }

    Vec6d bounds = m_Bounds;
    if (bounds.isZero())
    {
        Vec3d min, max;
        inputSurfaceMesh->computeBoundingBox(min, max, 0.0);
        bounds << min.x(), max.x(), min.y(), max.y(), min.z(), max.z();
        LOG(WARNING) << "SurfaceMeshDistanceTransform Bounds are zero, the input SurfaceMesh bounds (" << bounds.transpose() << ") will be used.";
    }

    const Vec3d size    = Vec3d(bounds[1] - bounds[0], bounds[3] - bounds[2], bounds[5] - bounds[4]);
    const Vec3d spacing = size.cwiseQuotient(m_Dimensions.cast<double>());
    const Vec3d origin  = Vec3d(bounds[0], bounds[2], bounds[4]);
    outputImageData->allocate(IMSTK_DOUBLE, 1, m_Dimensions, spacing, origin);

    /* StopWatch timer;
     timer.start();*/

    if (m_NarrowBanded)
    {
        computeNarrowBandedDT(outputImageData, inputSurfaceMesh, m_DilateSize);
    }
    else
    {
        computeFullDT(outputImageData, inputSurfaceMesh, m_Tolerance);
    }

    //printf("time: %f\n", timer.getTimeElapsed());
}
} // namespace imstk