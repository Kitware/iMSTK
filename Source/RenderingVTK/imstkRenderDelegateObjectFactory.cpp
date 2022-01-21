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
// Manually register creation functions
std::unordered_map<std::string, RenderDelegateObjectFactory::DelegateMakeFunc> RenderDelegateObjectFactory::m_objCreationMap =
    std::unordered_map<std::string, RenderDelegateObjectFactory::DelegateMakeFunc>();

std::shared_ptr<VTKRenderDelegate>
RenderDelegateObjectFactory::makeRenderDelegate(std::shared_ptr<VisualModel> visualModel)
{
    // If delegate hint is provided & it exists in map, override the creation of the delegate
    const std::string& delegateHint = visualModel->getDelegateHint();
    if (delegateHint != "" && m_objCreationMap.count(delegateHint) != 0)
    {
        return m_objCreationMap.at(delegateHint)(visualModel);
    }

    // Otherwise use the usual creation scheme
    std::shared_ptr<Geometry> geom = visualModel->getGeometry();
    if (geom != nullptr)
    {
        // Two edge cases
        if (visualModel->getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Fluid)
        {
            if (std::dynamic_pointer_cast<PointSet>(geom) != nullptr)
            {
                return std::make_shared<VTKFluidRenderDelegate>(visualModel);
            }
        }
        if (visualModel->getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::SurfaceNormals)
        {
            if (std::dynamic_pointer_cast<PointSet>(geom) != nullptr)
            {
                return std::make_shared<VTKSurfaceNormalRenderDelegate>(visualModel);
            }
        }

        if (visualModel->getGeometry()->isMesh())
        {
            if (std::dynamic_pointer_cast<SurfaceMesh>(geom) != nullptr)
            {
                return std::make_shared<VTKSurfaceMeshRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<TetrahedralMesh>(geom) != nullptr)
            {
                return std::make_shared<VTKTetrahedralMeshRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<LineMesh>(geom) != nullptr)
            {
                return std::make_shared<VTKLineMeshRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<HexahedralMesh>(geom) != nullptr)
            {
                return std::make_shared<VTKHexahedralMeshRenderDelegate>(visualModel);
            }
        }
        else
        {
            if (std::dynamic_pointer_cast<PointSet>(geom) != nullptr)
            {
                return std::make_shared<VTKPointSetRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<Plane>(geom) != nullptr)
            {
                return std::make_shared<VTKPlaneRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<Sphere>(geom) != nullptr)
            {
                return std::make_shared<VTKSphereRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<Capsule>(geom) != nullptr)
            {
                return std::make_shared<VTKCapsuleRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<OrientedBox>(geom) != nullptr)
            {
                return std::make_shared<VTKOrientedCubeRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<Cylinder>(geom) != nullptr)
            {
                return std::make_shared<VTKCylinderRenderDelegate>(visualModel);
            }
            else if (std::dynamic_pointer_cast<ImageData>(geom) != nullptr)
            {
                if (visualModel->getRenderMaterial()->getDisplayMode() == RenderMaterial::DisplayMode::Points)
                {
                    return std::make_shared<VTKPointSetRenderDelegate>(visualModel);
                }
                else
                {
                    return std::make_shared<VTKImageDataRenderDelegate>(visualModel);
                }
            }
        }
    }
    LOG(FATAL) << "RenderDelegate::makeDelegate error: Could not find delegate for rendering";
    return nullptr;
}
}