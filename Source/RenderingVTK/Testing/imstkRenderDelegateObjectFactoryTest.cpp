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

#pragma once

#include "imstkSurfaceMesh.h"
#include "imstkRenderDelegateObjectFactory.h"
#include "imstkRenderMaterial.h"
#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkVTKRenderDelegate.h"
#include "imstkVTKSurfaceNormalRenderDelegate.h"
#include "imstkVisualModel.h"

#include "gtest/gtest.h"

namespace imstk
{
class RenderDelegateMock : public VTKRenderDelegate
{
public:
    RenderDelegateMock(std::shared_ptr<VisualModel> visualModel) : VTKRenderDelegate(visualModel)
    {
    }

    ~RenderDelegateMock() override = default;

    void updateRenderProperties() override { }
};

TEST(imstkRenderDelegateFactoryTest, CustomRenderDelegate)
{
    REGISTER_RENDER_DELEGATE(RenderDelegateMock);

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setDelegateHint("RenderDelegateMock");
    std::shared_ptr<VTKRenderDelegate> renderDelegate =
        RenderDelegateObjectFactory::makeRenderDelegate(visualModel);

    // Expect RenderDelegateMock to be created when given a visualModel
    EXPECT_TRUE(std::dynamic_pointer_cast<RenderDelegateMock>(renderDelegate) != nullptr);
}

TEST(imstkRenderDelegateFactoryTest, GeometryRenderDelegate)
{
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(std::make_shared<SurfaceMesh>());

    std::shared_ptr<VTKRenderDelegate> renderDelegate =
        RenderDelegateObjectFactory::makeRenderDelegate(visualModel);

    EXPECT_TRUE(std::dynamic_pointer_cast<VTKSurfaceMeshRenderDelegate>(renderDelegate) != nullptr);
}

TEST(imstkRenderDelegateFactoryTest, MaterialRenderDelegate)
{
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(std::make_shared<SurfaceMesh>());
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    visualModel->setRenderMaterial(material);

    std::shared_ptr<VTKRenderDelegate> renderDelegate =
        RenderDelegateObjectFactory::makeRenderDelegate(visualModel);

    EXPECT_TRUE(std::dynamic_pointer_cast<VTKSurfaceNormalRenderDelegate>(renderDelegate) != nullptr);
}
} // namespace imstk