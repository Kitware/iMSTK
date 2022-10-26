/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKCapsuleRenderDelegate.h"
#include "imstkCapsule.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkCapsuleSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>

namespace imstk
{
VTKCapsuleRenderDelegate::VTKCapsuleRenderDelegate() :
    m_capsuleSource(vtkSmartPointer<vtkCapsuleSource>::New())
{
}

void
VTKCapsuleRenderDelegate::init()
{
    auto geometry = std::dynamic_pointer_cast<Capsule>(m_visualModel->getGeometry());
    CHECK(geometry != nullptr) << "VTKCapsuleRenderDelegate only works with Capsule geometry";

    m_capsuleSource->SetCenter(0.0, 0.0, 0.0);
    m_capsuleSource->SetRadius(geometry->getRadius(Geometry::DataType::PreTransform));
    m_capsuleSource->SetCylinderLength(geometry->getLength(Geometry::DataType::PreTransform));
    m_capsuleSource->SetLatLongTessellation(20);
    m_capsuleSource->SetPhiResolution(20);
    m_capsuleSource->SetThetaResolution(20);

    const Mat4d& transform = geometry->getTransform().transpose();
    m_transform->SetMatrix(transform.data());

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(m_capsuleSource->GetOutputPort());
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
VTKCapsuleRenderDelegate::processEvents()
{
    VTKRenderDelegate::processEvents();

    // Don't use events for primitives, just always update
    auto geometry = std::dynamic_pointer_cast<Capsule>(m_visualModel->getGeometry());

    m_capsuleSource->SetRadius(geometry->getRadius(Geometry::DataType::PreTransform));
    m_capsuleSource->SetCylinderLength(geometry->getLength(Geometry::DataType::PreTransform));
    m_capsuleSource->Modified();

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(Geometry::DataType::PostTransform));
    T.rotate(geometry->getOrientation(Geometry::DataType::PostTransform));
    T.scale(geometry->getScaling());
    T.matrix().transposeInPlace();
    m_transform->SetMatrix(T.data());
}
} // namespace imstk