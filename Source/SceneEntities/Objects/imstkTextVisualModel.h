/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVisualModel.h"
#include "imstkColor.h"

namespace imstk
{
///
/// \class TextVisualModel
///
/// \brief Renders text to the screen
///

class TextVisualModel : public VisualModel
{
public:

    enum class DisplayPosition
    {
        LowerLeft,
        UpperLeft,
        UpperRight,
        LowerRight,
        CenterCenter
    };

    TextVisualModel(const std::string& name = "TextVisualModel") : VisualModel(name)
    {
        setDelegateHint("TextRenderDelegate");
    }

    ///
    /// \brief Text to be plotted
    ///@{
    void setText(const std::string& text) { m_text = text; }
    const std::string& getText() const { return m_text; }
    ///@}

    ///
    /// \brief Text Color
    ///@{
    const Color& getTextColor() const { return m_textColor; }
    void setTextColor(const Color& color) { m_textColor = color; }
    ///@}

    ///
    /// \brief Font size
    ///@{
    double getFontSize() const { return m_fontSize; }
    void setFontSize(const double fontSize) { m_fontSize = fontSize; }
    ///@}

    ///
    /// \brief Text visibility
    ///@{
    void setVisibility(const bool visible) { m_visible = visible; }
    const bool getVisibility() const { return m_visible; }
    ///@}

    ///
    /// \brief Text Location
    ///@{
    void setPosition(const DisplayPosition position) { m_position = position; }
    DisplayPosition getPosition() const { return m_position; }
///@}

protected:
    std::string m_text      = "";
    Color       m_textColor = Color::White;
    double      m_fontSize  = 20.0;
    bool m_visible = true;

    DisplayPosition m_position = DisplayPosition::CenterCenter;
};
} // namespace imstk