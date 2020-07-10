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

#include "imstkSurfaceMeshImageMask.h"
#include "imstkCleanMesh.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"

#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>

namespace imstk
{
SurfaceMeshImageMask::SurfaceMeshImageMask()
{
    setNumberOfInputPorts(2);
    setNumberOfOutputPorts(1);
    setOutput(std::make_shared<ImageData>(), 0);
}

void
SurfaceMeshImageMask::setReferenceImage(std::shared_ptr<ImageData> refImage)
{
    setInput(refImage, 1);
}

void
SurfaceMeshImageMask::setInputMesh(std::shared_ptr<SurfaceMesh> mesh)
{
    setInput(mesh, 0);
}

std::shared_ptr<ImageData>
SurfaceMeshImageMask::getOutputImage() const
{
    return std::static_pointer_cast<ImageData>(getOutput(0));
}

void
SurfaceMeshImageMask::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> surfMeshInput = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    std::shared_ptr<ImageData>   refImageInput = std::dynamic_pointer_cast<ImageData>(getInput(1));

    if (surfMeshInput == nullptr)
    {
        LOG(WARNING) << "Missing input surface mesh";
        return;
    }

    double spacing[3];
    int    extent[6];
    double origin[3];
    if (refImageInput != nullptr)
    {
        const Vec3d& refSpacing = refImageInput->getSpacing();
        spacing[0] = refSpacing.x();
        spacing[1] = refSpacing.y();
        spacing[2] = refSpacing.z();

        const Vec3d& refOrigin = refImageInput->getOrigin();
        origin[0] = refOrigin[0];
        origin[1] = refOrigin[1];
        origin[2] = refOrigin[2];

        const Vec3i& dim = refImageInput->getDimensions();
        extent[0] = 0;
        extent[1] = dim[0];
        extent[2] = 0;
        extent[3] = dim[1];
        extent[4] = 0;
        extent[5] = dim[2];
    }
    else
    {
        if (Dimensions[0] == -1 || Dimensions[1] == -1 || Dimensions[2] == -1)
        {
            LOG(WARNING) << "No reference image or desired image dimensions.";
            return;
        }
        Vec3d min;
        Vec3d max;
        surfMeshInput->computeBoundingBox(min, max);
        double bounds[6] =
        {
            min.x(), max.x(),
            min.y(), max.y(),
            min.z(), max.z()
        };
        double size[3] = {
            bounds[1] - bounds[0],
            bounds[3] - bounds[2],
            bounds[5] - bounds[4] };
        // Spacing required leaving room for extent
        // ie: User will still get 100x100x100 around their objects bounds, bounds will be slightly increased
        spacing[0] = size[0] / Dimensions[0];
        spacing[1] = size[1] / Dimensions[1];
        spacing[2] = size[2] / Dimensions[2];

        // Increase bounds by px length
        bounds[0] -= spacing[0] * BorderExtent;
        bounds[1] += spacing[0] * BorderExtent;
        bounds[2] -= spacing[1] * BorderExtent;
        bounds[3] += spacing[1] * BorderExtent;
        bounds[4] -= spacing[2] * BorderExtent;
        bounds[5] += spacing[2] * BorderExtent;

        extent[0] = 0;
        extent[1] = static_cast<int>((bounds[1] - bounds[0]) / spacing[0]) - 1;
        extent[2] = 0;
        extent[3] = static_cast<int>((bounds[3] - bounds[2]) / spacing[1]) - 1;
        extent[4] = 0;
        extent[5] = static_cast<int>((bounds[5] - bounds[4]) / spacing[2]) - 1;
        origin[0] = bounds[0] + spacing[0] * 0.5;
        origin[1] = bounds[2] + spacing[1] * 0.5;
        origin[2] = bounds[4] + spacing[2] * 0.5;
    }

    // Allocate a new white image
    vtkSmartPointer<vtkImageData> whiteImage = vtkSmartPointer<vtkImageData>::New();
    whiteImage->SetSpacing(spacing);
    whiteImage->SetExtent(extent);
    whiteImage->SetOrigin(origin);
    whiteImage->AllocateScalars(VTK_FLOAT, 1);
    int* dim = whiteImage->GetDimensions();
    std::fill_n(static_cast<float*>(whiteImage->GetScalarPointer()), dim[0] * dim[1] * dim[2], 1.0f);

    // Creates a new image mask from this polygon using a reference mask
    vtkSmartPointer<vtkPolyDataToImageStencil> poly2Stencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    poly2Stencil->SetInputData(GeometryUtils::copyToVtkPolyData(surfMeshInput));
    poly2Stencil->SetOutputOrigin(origin);
    poly2Stencil->SetOutputSpacing(spacing);
    poly2Stencil->SetOutputWholeExtent(extent);
    poly2Stencil->Update();
    vtkSmartPointer<vtkImageStencil> imgStencil = vtkSmartPointer<vtkImageStencil>::New();
    imgStencil->SetInputData(whiteImage);
    imgStencil->SetStencilData(poly2Stencil->GetOutput());
    imgStencil->ReverseStencilOff();
    imgStencil->SetBackgroundValue(0.0);
    imgStencil->Update();

    // Set the output
    setOutput(GeometryUtils::copyToImageData(imgStencil->GetOutput()));
}
}