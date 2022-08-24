/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKRenderDelegate.h"

class vtkActor2D;
class vtkTextMapper;

namespace imstk
{
///
/// \class VTKTextRenderDelegate
///
/// \brief Render delegate for text on the screen
///
class VTKTextRenderDelegate : public VTKRenderDelegate
{
public:
    VTKTextRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKTextRenderDelegate() override = default;

    ///
    /// \brief Plot text and track updates to TextVisualModel
    ///
    void processEvents() override;

    void updateRenderProperties() override;

protected:
    vtkSmartPointer<vtkActor2D>  m_textActor;
    vtkSmartPointer<vtkTextMapper> m_textMapper;
};
} // namespace imstk