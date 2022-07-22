/*=========================================================================

    Library: iMSTK

    Copyright (c) Kitware

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0.txt

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

=========================================================================*/

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

    TextVisualModel()
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
    void setVisability(const bool visible) { m_visible = visible; }
    const bool getVisability() const { return m_visible; }
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
    bool m_visible = false;

    DisplayPosition m_position = DisplayPosition::CenterCenter;
};
} // namespace imstk