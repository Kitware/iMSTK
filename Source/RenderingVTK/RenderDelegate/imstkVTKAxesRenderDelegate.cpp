/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKAxesRenderDelegate.h"
#include "imstkAxesModel.h"
#include "imstkLogger.h"

#include <vtkAbstractMapper.h>
#include <vtkAxesActor.h>
#include <vtkTransform.h>

namespace imstk
{
VTKAxesRenderDelegate::VTKAxesRenderDelegate(
    std::shared_ptr<VisualModel> visualModel) :
    VTKRenderDelegate(visualModel)
{
    auto model = std::dynamic_pointer_cast<AxesModel>(visualModel);
    CHECK(model != nullptr)
        << "VTKAxesRenderDelegate created with non-AxesModel";

    const Mat4d transform =
        (mat4dTranslate(model->getPosition()) *
         mat4dRotation(model->getOrientation())).transpose();
    m_transform->SetMatrix(transform.data());

    // Setup mapper
    {
        m_axesActor = vtkSmartPointer<vtkAxesActor>::New();
        m_axesActor->SetShaftType(vtkAxesActor::CYLINDER_SHAFT);
        m_axesActor->SetAxisLabels(model->getAxesLabelsVisible());
        const Vec3d& scale = model->getScale();
        m_axesActor->SetTotalLength(scale[0], scale[1], scale[2]);
        m_axesActor->SetUserTransform(m_transform);
        m_mapper = nullptr;
        m_actor  = m_axesActor;
    }

    update();
    updateRenderProperties();
}

void
VTKAxesRenderDelegate::processEvents()
{
    VTKRenderDelegate::processEvents();

    auto model = std::dynamic_pointer_cast<AxesModel>(m_visualModel);
    if (static_cast<bool>(m_axesActor->GetVisibility()) != model->isVisible())
    {
        m_axesActor->SetVisibility(model->isVisible());
    }

    if (model->isVisible())
    {
        if (static_cast<bool>(m_axesActor->GetAxisLabels()) != model->getAxesLabelsVisible())
        {
            m_axesActor->SetAxisLabels(model->getAxesLabelsVisible());
        }

        const Mat4d transform =
            (mat4dTranslate(model->getPosition()) *
             mat4dRotation(model->getOrientation())).transpose();
        m_transform->SetMatrix(transform.data());
        const Vec3d& scale = model->getScale();
        m_axesActor->SetTotalLength(scale[0], scale[1], scale[2]);
    }
}
} // namespace imstk