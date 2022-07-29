/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>

#include "imstkVTKTextRenderDelegate.h"
#include "imstkTextVisualModel.h"

namespace imstk
{
VTKTextRenderDelegate::VTKTextRenderDelegate(std::shared_ptr<VisualModel> visualModel)
    : VTKRenderDelegate(visualModel)
    , m_textActor(vtkSmartPointer<vtkTextActor>::New()), m_textMapper(vtkSmartPointer<vtkTextMapper>::New())
{
    auto textVisualModel = std::dynamic_pointer_cast<TextVisualModel>(visualModel);

    m_visualModel = visualModel;

    m_textMapper->SetInput(textVisualModel->getText().c_str());

    // Pull properties from textVisualModel
    auto* txtprop = m_textMapper->GetTextProperty();
    txtprop->SetFontSize(textVisualModel->getFontSize());
    txtprop->SetFontFamilyToArial();
    txtprop->SetBackgroundColor(180, 180, 180);
    txtprop->SetBackgroundOpacity(0.75);

    const Color& fontColor = textVisualModel->getTextColor();
    txtprop->SetColor(fontColor.r, fontColor.g, fontColor.b);
    txtprop->SetJustificationToCentered();
    txtprop->SetVerticalJustificationToCentered();

    // Setup Text Actor
    m_textActor->SetInput(textVisualModel->getText().c_str());
    m_textActor->SetMapper(m_textMapper);
    m_textActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    m_textActor->GetPositionCoordinate()->SetValue(0.5, 0.5);

    // Set text position
    TextVisualModel::DisplayPosition position = textVisualModel->getPosition();
    switch (position)
    {
    case TextVisualModel::DisplayPosition::CenterCenter:
        // Use the default value
        break;

    case TextVisualModel::DisplayPosition::UpperLeft:
        txtprop->SetJustificationToLeft();
        txtprop->SetVerticalJustificationToTop();
        m_textActor->GetPositionCoordinate()->SetValue(0.01, 0.95);
        break;

    case TextVisualModel::DisplayPosition::UpperRight:
        txtprop->SetJustificationToRight();
        txtprop->SetVerticalJustificationToTop();
        m_textActor->GetPositionCoordinate()->SetValue(0.95, 0.95);
        break;

    case TextVisualModel::DisplayPosition::LowerRight:
        txtprop->SetJustificationToRight();
        txtprop->SetVerticalJustificationToBottom();
        m_textActor->GetPositionCoordinate()->SetValue(0.95, 0.05);
        break;

    case TextVisualModel::DisplayPosition::LowerLeft:
        txtprop->SetJustificationToLeft();
        txtprop->SetVerticalJustificationToBottom();
        m_textActor->GetPositionCoordinate()->SetValue(0.05, 0.05);
        break;
    }

    m_actor = m_textActor;

    processEvents();
}

void
VTKTextRenderDelegate::processEvents()
{
    // Update text if changed
    auto textVisualModel = std::dynamic_pointer_cast<TextVisualModel>(m_visualModel);

    if (textVisualModel->getText() != std::string(m_textActor->GetInput()))
    {
        m_textActor->SetInput(textVisualModel->getText().c_str());
    }

    m_textActor->SetVisibility(textVisualModel->getVisibility());
}

void
VTKTextRenderDelegate::updateRenderProperties()
{
}
} // namespace imstk