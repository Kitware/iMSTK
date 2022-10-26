/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKCylinderRenderDelegate.h"
#include "imstkCylinder.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>

namespace imstk
{
VTKCylinderRenderDelegate::VTKCylinderRenderDelegate() :
    m_cylinderSource(vtkSmartPointer<vtkCylinderSource>::New())
{
}

void
VTKCylinderRenderDelegate::init()
{
    auto geometry = std::dynamic_pointer_cast<Cylinder>(m_visualModel->getGeometry());
    CHECK(geometry != nullptr) << "VTKCylinderRenderDelegate only works with Cylinder geometry";

    m_cylinderSource->SetCenter(0.0, 0.0, 0.0);
    m_cylinderSource->SetRadius(geometry->getRadius(Geometry::DataType::PreTransform));
    m_cylinderSource->SetHeight(geometry->getLength(Geometry::DataType::PreTransform));
    m_cylinderSource->SetResolution(100);

    const Mat4d& transform = geometry->getTransform().transpose();
    m_transform->SetMatrix(transform.data());

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(m_cylinderSource->GetOutputPort());
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
VTKCylinderRenderDelegate::processEvents()
{
    VTKRenderDelegate::processEvents();

    auto geometry = std::dynamic_pointer_cast<Cylinder>(m_visualModel->getGeometry());

    m_cylinderSource->SetRadius(geometry->getRadius(Geometry::DataType::PreTransform));
    m_cylinderSource->SetHeight(geometry->getLength(Geometry::DataType::PreTransform));
    m_cylinderSource->Modified();

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(Geometry::DataType::PostTransform));
    T.rotate(geometry->getOrientation(Geometry::DataType::PostTransform));
    T.scale(geometry->getScaling());
    T.matrix().transposeInPlace();
    m_transform->SetMatrix(T.data());
}
} // namespace imstk