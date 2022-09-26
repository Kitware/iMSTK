/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
#include "imstkVTKAxesRenderDelegate.h"
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
#include "imstkVTKTextRenderDelegate.h"
#include "imstkVTKVertexLabelRenderDelegate.h"

namespace imstk
{
///
/// \brie Note, these do not refer to geometry classes
///
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
IMSTK_REGISTER_RENDERDELEGATE(VertexLabel, VTKVertexLabelRenderDelegate)
IMSTK_REGISTER_RENDERDELEGATE(TextRenderDelegate, VTKTextRenderDelegate)

// Custom algorithms
RenderDelegateRegistrar<VTKAxesRenderDelegate>          _imstk_registerrenderdelegate_axes("AxesModel");
RenderDelegateRegistrar<VTKFluidRenderDelegate>         _imstk_registerrenderdelegate_fluid("Fluid");
RenderDelegateRegistrar<VTKSurfaceNormalRenderDelegate> _imstk_registerrenderdelegate_surfacenormals("SurfaceNormals");

std::shared_ptr<VTKRenderDelegate>
RenderDelegateObjectFactory::makeRenderDelegate(std::shared_ptr<VisualModel> visualModel)
{
    if (visualModel == nullptr)
    {
        LOG(FATAL) << "RenderDelegate::makeDelegate error: Called with null visualModel";
        return nullptr;
    }

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