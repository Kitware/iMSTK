/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKPlaneRenderDelegate.h"
#include "imstkPlane.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>

namespace imstk
{
VTKPlaneRenderDelegate::VTKPlaneRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel),
    m_planeSource(vtkSmartPointer<vtkPlaneSource>::New())
{
    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(m_planeSource->GetOutputPort());
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        actor->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = actor;
    }

    update();
    updateRenderProperties();

    // Call to update to initialize
    processEvents();
}

void
VTKPlaneRenderDelegate::processEvents()
{
    VTKRenderDelegate::processEvents();

    // Events aren't used for primitives, always update
    auto geometry = std::dynamic_pointer_cast<Plane>(m_visualModel->getGeometry());

    m_planeSource->SetCenter(0.0, 0.0, 0.0);
    m_planeSource->SetNormal(geometry->getNormal(Geometry::DataType::PreTransform).data());
    m_planeSource->Modified();

    AffineTransform3d T = AffineTransform3d::Identity();
    T.translate(geometry->getPosition(Geometry::DataType::PostTransform));
    T.rotate(geometry->getOrientation(Geometry::DataType::PostTransform));
    T.scale(geometry->getWidth());
    T.matrix().transposeInPlace();
    m_transform->SetMatrix(T.data());
}
} // namespace imstk