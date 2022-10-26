/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    RenderDelegateMock() = default;
    ~RenderDelegateMock() override = default;

    void updateRenderProperties() override { }

protected:
    void init() override { }
};

TEST(imstkRenderDelegateFactoryTest, CustomRenderDelegate)
{
    RenderDelegateRegistrar<RenderDelegateMock> registerMockDelegate("MockDelegateHint");

    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setDelegateHint("MockDelegateHint");
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