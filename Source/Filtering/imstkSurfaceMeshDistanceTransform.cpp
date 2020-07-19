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
//static bool neighborNotEqual(const Vec3i& pt, const Vec3i& dim, const float val, const float* imgPtr)
//{
//    const Vec3i ptc = pt.cwiseMax(0).cwiseMin(dim - Vec3i(1, 1, 1));
//    const Vec3i minima = (pt - Vec3i(1, 1, 1)).cwiseMax(0).cwiseMin(dim - Vec3i(1, 1, 1));
//    const Vec3i maxima = (pt + Vec3i(1, 1, 1)).cwiseMax(0).cwiseMin(dim - Vec3i(1, 1, 1));
//
//    const size_t xIndex1 = ImageData::getScalarIndex(minima[0], ptc[1], ptc[2], dim, 1);
//    const size_t xIndex2 = ImageData::getScalarIndex(maxima[0], ptc[1], ptc[2], dim, 1);
//    const size_t yIndex1 = ImageData::getScalarIndex(ptc[0], minima[1], ptc[2], dim, 1);
//    const size_t yIndex2 = ImageData::getScalarIndex(ptc[0], maxima[1], ptc[2], dim, 1);
//    const size_t zIndex1 = ImageData::getScalarIndex(ptc[0], ptc[1], minima[2], dim, 1);
//    const size_t zIndex2 = ImageData::getScalarIndex(ptc[0], ptc[1], maxima[2], dim, 1);
//
//    if (imgPtr[xIndex1] != val)
//        return true;
//    if (imgPtr[xIndex2] != val)
//        return true;
//    if (imgPtr[yIndex1] != val)
//        return true;
//    if (imgPtr[yIndex2] != val)
//        return true;
//    if (imgPtr[zIndex1] != val)
//        return true;
//    if (imgPtr[zIndex2] == val)
//        return true;
//    return false;
//}

//// Narrow band is WIP, it works but is slow
//static void
//computeNarrowBandedDT(std::shared_ptr<ImageData> imageData, std::shared_ptr<SurfaceMesh> surfMesh, double bandwidth)
//{
//    // Rasterize a mask from the polygon
//    std::shared_ptr<SurfaceMeshImageMask> imageMask = std::make_shared<SurfaceMeshImageMask>();
//    imageMask->setInputMesh(surfMesh);
//    imageMask->setReferenceImage(imageData);
//    imageMask->update();
//
//    DataArray<float>& inputScalars = *std::dynamic_pointer_cast<DataArray<float>>(imageMask->getOutputImage()->getScalars());
//    float* inputImgPtr = inputScalars.getPointer();
//    DataArray<float>& outputScalars = *std::dynamic_pointer_cast<DataArray<float>>(imageData->getScalars());
//    float* outputImgPtr = outputScalars.getPointer();
//
//    vtkSmartPointer<vtkPolyData> inputPolyData = GeometryUtils::copyToVtkPolyData(surfMesh);
//    vtkSmartPointer<vtkImplicitPolyDataDistance> polyDataDist = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
//    polyDataDist->SetInput(inputPolyData);
//    std::fill_n(outputImgPtr, outputScalars.size(), VTK_FLOAT_MIN);
//
//    // Iterate the image testing for boundary pixels (ie any 0 adjacent to a 1)
//    const Vec3i& dim = imageData->getDimensions();
//    const Vec3d shift = imageData->getOrigin() + imageData->getSpacing() * 0.5;
//    const Vec3d& spacing = imageData->getSpacing();
//    int i = 0;
//    for (int z = 0; z < dim[2]; z++)
//    {
//        for (int y = 0; y < dim[1]; y++)
//        {
//            for (int x = 0; x < dim[0]; x++, i++)
//            {
//                const float val = inputImgPtr[i];
//                const Vec3i pt = Vec3i(x, y, z);
//                if (neighborNotEqual(pt, dim, val, inputImgPtr))
//                {
//                    const Vec3d pos = pt.cast<double>().cwiseProduct(spacing) + shift;
//                    const float dist = polyDataDist->FunctionValue(pos.data());
//                    //printf("%f\n", dist);
//                    outputImgPtr[i] = dist;
//                }
//                else if (val == 0.0f)
//                {
//                    outputImgPtr[i] = VTK_FLOAT_MIN;
//                }
//            }
//        }
//    }
//}

static void
computeFullDT(std::shared_ptr<ImageData> imageData, std::shared_ptr<SurfaceMesh> surfMesh)
{
    // Get the optimal number of threads
    const size_t numThreads = ParallelUtils::ThreadManager::getThreadPoolSize();

    const Vec6d& bounds  = imageData->getBounds();
    const Vec3i& dim     = imageData->getDimensions();
    const Vec3d  size    = Vec3d(bounds[1] - bounds[0], bounds[3] - bounds[2], bounds[5] - bounds[4]);
    const Vec3d  spacing = imageData->getSpacing();
    const Vec3d  shift   = imageData->getOrigin() + spacing * 0.5;

    DataArray<float>& scalars = *static_cast<DataArray<float>*>(imageData->getScalars().get());

    // Split the work up along z using thread count to avoid making too many octtrees (may not be most optimal)
    ParallelUtils::parallelFor(numThreads, [&](const size_t& i)
        {
            // Separate polygons used to avoid race conditions
            vtkSmartPointer<vtkPolyData> inputPolyData = GeometryUtils::copyToVtkPolyData(surfMesh);
            vtkSmartPointer<vtkImplicitPolyDataDistance> polyDataDist = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
            polyDataDist->SetInput(inputPolyData);
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
    setNumberOfInputPorts(1);
    setNumberOfOutputPorts(1);
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
SurfaceMeshDistanceTransform::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> inputSurfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    std::shared_ptr<ImageData>   outputImageData  = std::dynamic_pointer_cast<ImageData>(getOutput(0));

    if (inputSurfaceMesh == nullptr)
    {
        LOG(WARNING) << "Missing input surface mesh";
        return;
    }

    if (Dimensions[0] == 0 || Dimensions[1] == 0 || Dimensions[2] == 0)
    {
        LOG(WARNING) << "Dimensions not set";
        return;
    }

    // Allocate an image for it
    Vec3d min;
    Vec3d max;
    inputSurfaceMesh->computeBoundingBox(min, max);
    double bounds[6];
    if (UseBounds)
    {
        bounds[0] = Bounds[0];
        bounds[1] = Bounds[1];
        bounds[2] = Bounds[2];
        bounds[3] = Bounds[3];
        bounds[4] = Bounds[4];
        bounds[5] = Bounds[5];
    }
    else
    {
        bounds[0] = min[0];
        bounds[1] = max[0];
        bounds[2] = min[1];
        bounds[3] = max[1];
        bounds[4] = min[2];
        bounds[5] = max[2];
    }
    Vec3d size    = Vec3d(bounds[1] - bounds[0], bounds[3] - bounds[2], bounds[5] - bounds[4]);
    Vec3d spacing = size.cwiseQuotient(Dimensions.cast<double>());
    Vec3d origin  = Vec3d(bounds[0], bounds[2], bounds[4]);
    outputImageData->allocate(IMSTK_FLOAT, 1, Dimensions, spacing, origin);

    /* StopWatch timer;
     timer.start();*/

    /*if (NarrowBanded)
    {
        computeNarrowBandedDT(outputImageData, inputSurfaceMesh, NarrowBandWidth);
    }
    else*/
    {
        computeFullDT(outputImageData, inputSurfaceMesh);
    }

    //printf("time: %f\n", timer.getTimeElapsed());
}
}