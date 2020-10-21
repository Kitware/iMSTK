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

    Vec3d spacing;
    int   extent[6];
    Vec3d origin;
    if (refImageInput != nullptr)
    {
        spacing = refImageInput->getSpacing();

        origin = refImageInput->getOrigin();

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
        if (m_Dimensions[0] == -1 || m_Dimensions[1] == -1 || m_Dimensions[2] == -1)
        {
            LOG(WARNING) << "No reference image or desired image dimensions.";
            return;
        }
        Vec3d min;
        Vec3d max;
        surfMeshInput->computeBoundingBox(min, max);

        Vec3d size = max - min;

        // Compute spacing required for given dimension
        spacing = size.cwiseQuotient(m_Dimensions.cast<double>());

        // Increase bounds by px length
        min -= spacing * m_BorderExtent;
        max += spacing * m_BorderExtent;

        size = max - min;

        extent[0] = 0;
        extent[1] = (m_Dimensions[0] + m_BorderExtent) - 1;
        extent[2] = 0;
        extent[3] = (m_Dimensions[1] + m_BorderExtent) - 1;
        extent[4] = 0;
        extent[5] = (m_Dimensions[2] + m_BorderExtent) - 1;

        origin = min - m_BorderExtent * spacing;
    }

    // Allocate a new white image
    vtkNew<vtkImageData> baseImage;
    baseImage->SetSpacing(spacing.data());
    baseImage->SetExtent(extent);
    baseImage->SetOrigin(origin.data());
    baseImage->AllocateScalars(VTK_FLOAT, 1);
    int* dim = baseImage->GetDimensions();
    std::fill_n(static_cast<float*>(baseImage->GetScalarPointer()), dim[0] * dim[1] * dim[2], 1.0f);

    // Creates a new image mask from this polygon using a reference mask
    vtkNew<vtkPolyDataToImageStencil> poly2Stencil;
    poly2Stencil->SetInputData(GeometryUtils::copyToVtkPolyData(surfMeshInput));
    poly2Stencil->SetOutputOrigin(origin.data());
    poly2Stencil->SetOutputSpacing(spacing.data());
    poly2Stencil->SetOutputWholeExtent(extent);
    poly2Stencil->Update();
    vtkNew<vtkImageStencil> imgStencil;
    imgStencil->SetInputData(baseImage);
    imgStencil->SetStencilData(poly2Stencil->GetOutput());
    imgStencil->ReverseStencilOff();
    imgStencil->SetBackgroundValue(0.0);
    imgStencil->Update();

    // Set the output
    setOutput(GeometryUtils::copyToImageData(imgStencil->GetOutput()));
}
}