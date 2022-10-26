/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKOrientedBoxRenderDelegate.h"
#include "imstkOrientedBox.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>

namespace imstk
{
VTKOrientedCubeRenderDelegate::VTKOrientedCubeRenderDelegate() :
    m_cubeSource(vtkSmartPointer<vtkCubeSource>::New())
{
}

void
VTKOrientedCubeRenderDelegate::init()
{
    auto geometry = std::dynamic_pointer_cast<OrientedBox>(m_visualModel->getGeometry());
    CHECK(geometry != nullptr) << "VTKOrientedCubeRenderDelegate only works with OrientedBox geometry";

    const Vec3d& extents = geometry->getExtents(Geometry::DataType::PreTransform);
    m_cubeSource->SetCenter(0.0, 0.0, 0.0);
    m_cubeSource->SetXLength(extents[0] * 2.0);
    m_cubeSource->SetYLength(extents[1] * 2.0);
    m_cubeSource->SetZLength(extents[2] * 2.0);

    const Mat4d& transform = geometry->getTransform().transpose();
    m_transform->SetMatrix(transform.data());

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(m_cubeSource->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = actor;
    }

    update();
    updateRenderProperties();
}

void
VTKOrientedCubeRenderDelegate::processEvents()
{
    VTKRenderDelegate::processEvents();

    // Events aren't used for primitives, always update
    auto geometry = std::dynamic_pointer_cast<OrientedBox>(m_visualModel->getGeometry());

    const Vec3d& extents = geometry->getExtents(Geometry::DataType::PreTransform);
    m_cubeSource->SetXLength(extents[0] * 2.0);
    m_cubeSource->SetYLength(extents[1] * 2.0);
    m_cubeSource->SetZLength(extents[2] * 2.0);
    m_cubeSource->Modified();

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(Geometry::DataType::PostTransform));
    T.rotate(geometry->getOrientation(Geometry::DataType::PostTransform));
    T.scale(geometry->getScaling());
    T.matrix().transposeInPlace();
    m_transform->SetMatrix(T.data());
}
} // namespace imstk