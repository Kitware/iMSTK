/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKSphereRenderDelegate.h"
#include "imstkSphere.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>

namespace imstk
{
VTKSphereRenderDelegate::VTKSphereRenderDelegate() :
    m_sphereSource(vtkSmartPointer<vtkSphereSource>::New())
{
}

void
VTKSphereRenderDelegate::init()
{
    auto geometry = std::dynamic_pointer_cast<Sphere>(m_visualModel->getGeometry());
    CHECK(geometry != nullptr) << "VTKSphereRenderDelegate only works with Sphere geometry";

    m_sphereSource->SetCenter(0.0, 0.0, 0.0);
    m_sphereSource->SetRadius(geometry->getRadius(Geometry::DataType::PreTransform));
    m_sphereSource->SetPhiResolution(20);
    m_sphereSource->SetThetaResolution(20);

    const Mat4d& transform = geometry->getTransform().transpose();
    m_transform->SetMatrix(transform.data());

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(m_sphereSource->GetOutputPort());
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
VTKSphereRenderDelegate::processEvents()
{
    VTKRenderDelegate::processEvents();

    auto geometry = std::dynamic_pointer_cast<Sphere>(m_visualModel->getGeometry());

    m_sphereSource->SetRadius(geometry->getRadius(Geometry::DataType::PreTransform));
    m_sphereSource->Modified();

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(Geometry::DataType::PostTransform));
    T.rotate(geometry->getOrientation(Geometry::DataType::PostTransform));
    T.scale(geometry->getScaling().maxCoeff());
    T.matrix().transposeInPlace();
    m_transform->SetMatrix(T.data());
}
} // namespace imstk