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
/// \class VertexLabelVisualModel
///
/// \brief Given a PointSet geometry it will render labels for each vertex with
/// numberings
///
class VertexLabelVisualModel : public VisualModel
{
public:
    VertexLabelVisualModel()
    {
        setDelegateHint("VertexLabel");
    }

    ///
    /// \brief Get/Set printf style format string for every vertex
    /// %d for integer
    /// @{
    const std::string& getFormat() const { return m_format; }
    void setFormat(const std::string& format) { m_format = format; }
    /// @}

    ///
    /// \brief Get/Set the text color
    /// @{
    const Color& getTextColor() const { return m_textColor; }
    void setTextColor(const Color& color) { m_textColor = color; }
    /// @}

    ///
    /// \brief Get/Set the font size
    /// @{
    double getFontSize() const { return m_fontSize; }
    void setFontSize(const double fontSize) { m_fontSize = fontSize; }
/// @}

protected:
    std::string m_format    = "%d";
    Color       m_textColor = Color::White;
    double      m_fontSize  = 10.0;
};
} // namespace imstk