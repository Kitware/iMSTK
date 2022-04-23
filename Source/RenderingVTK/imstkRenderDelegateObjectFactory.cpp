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

#include "imstkRenderDelegateObjectFactory.h"
#include "imstkCapsule.h"
#include "imstkCylinder.h"
#include "imstkGeometry.h"
#include "imstkHexahedralMesh.h"
#include "imstkImageData.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkOrientedBox.h"
#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKCapsuleRenderDelegate.h"
#include "imstkVTKCylinderRenderDelegate.h"
#include "imstkVTKFluidRenderDelegate.h"
#include "imstkVTKHexahedralMeshRenderDelegate.h"
#include "imstkVTKImageDataRenderDelegate.h"
#include "imstkVTKLineMeshRenderDelegate.h"
#include "imstkVTKOrientedBoxRenderDelegate.h"
#include "imstkVTKPlaneRenderDelegate.h"
#include "imstkVTKPointSetRenderDelegate.h"
#include "imstkVTKSphereRenderDelegate.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkVTKSurfaceNormalRenderDelegate.h"
#include "imstkVTKTetrahedralMeshRenderDelegate.h"

namespace imstk
{
IMSTK_REGISTER_RENDERDELEGATE(SurfaceMesh, VTKSurfaceMeshRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(Cylinder, VTKCylinderRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(TetrahedralMesh, VTKTetrahedralMeshRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(LineMesh, VTKLineMeshRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(HexahedralMesh, VTKHexahedralMeshRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(PointSet, VTKPointSetRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(Plane, VTKPlaneRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(Sphere, VTKSphereRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(Capsule, VTKCapsuleRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(OrientedBox, VTKOrientedCubeRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(ImageData, VTKImageDataRenderDelegate)

// Custom algorithms
RenderDelegateRegistrar<VTKFluidRenderDelegate>         _imstk_registerrenderdelegate_fluid("Fluid");
RenderDelegateRegistrar<VTKSurfaceNormalRenderDelegate> _imstk_registerrenderdelegate_surfacenormals("SurfaceNormals");

std::shared_ptr<VTKRenderDelegate>
RenderDelegateObjectFactory::makeRenderDelegate(std::shared_ptr<VisualModel> visualModel)
{
    // If delegate hint is provided & it exists in map, override the creation of the delegate
    const std::string& delegateHint = visualModel->getDelegateHint();
    if (delegateHint == "")
    {
        LOG(FATAL) << "RenderDelegate::makeDelegate error: No hint defined for visualModel "
                   << visualModel->getName();
        return nullptr;
    }

    if (!contains(delegateHint))
    {
        LOG(FATAL) << "RenderDelegate::makeDelegate error: can't find delegate with hint: "
                   << delegateHint << " for visual model " << visualModel->getName();
        return nullptr;
    }

    // Still a bug, should be able to copy the visual model ptr as well
    return create(delegateHint, std::move(visualModel));
}
} // namespace imstk