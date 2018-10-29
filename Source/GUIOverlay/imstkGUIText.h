/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

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

#ifndef imstkText_h
#define imstkText_h

#include "imgui.h"

#include <string>

#include "imstkGUIWidget.h"
#include "imstkColor.h"
#include "imstkGUIUtilities.h"

namespace imstk
{
namespace GUIOverlay
{
///
/// \class Text
///
/// \brief Text widget
///
class Text : public Widget
{
public:
    ///
    /// \brief Protected constructor
    ///
    Text(std::string name,
         std::string value,
         float x,
         float y,
         float fontSize = 14.0f)
        : Widget(name, x, y)
    {
        m_value = value;
        m_fontSize = fontSize;
        m_type = Widget::Type::Text;
    }

    ///
    /// \brief Render
    ///
    virtual void render(const bool inWindow)
    {
        if (inWindow)
        {
            ImGui::Text(m_value.c_str(),
                ImVec2(m_position[0], m_position[1]),
                m_fontSize,
                GUIOverlay::Utilities::convertToGUIColor(m_color));
        }
        else
        {
            auto drawList = ImGui::GetOverlayDrawList();
            drawList->AddText(ImGui::GetFont(),
                m_fontSize,
                ImVec2(m_position[0], m_position[1]),
                GUIOverlay::Utilities::convertToGUIColor(m_color),
                m_value.c_str());
        }
    }

    ///
    /// \brief Set text
    ///
    void setText(const std::string text)
    {
        m_value = text;
    }

    ///
    /// \brief Get text
    ///
    const std::string& getText()
    {
        return m_value;
    }

protected:
    Text() = delete;

    std::string m_value;
    float m_fontSize = 10;
    Color m_color = Color::White;
};
} // GUI
} // imstk

#endif // ifndef imstkWidget_h
