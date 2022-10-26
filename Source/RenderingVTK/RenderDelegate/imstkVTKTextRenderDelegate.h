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
/// \brief Delegates rendering of text to VTK from TextVisualModel
///
class VTKTextRenderDelegate : public VTKRenderDelegate
{
public:
    VTKTextRenderDelegate() = default;
    ~VTKTextRenderDelegate() override = default;

    ///
    /// \brief Plot text and track updates to TextVisualModel
    ///
    void processEvents() override;

    void updateRenderProperties() override;

protected:
    void init() override;

    vtkSmartPointer<vtkActor2D>    m_textActor;
    vtkSmartPointer<vtkTextMapper> m_textMapper;
};
} // namespace imstk