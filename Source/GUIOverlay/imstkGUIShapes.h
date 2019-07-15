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

#pragma once

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
/// \class Shape
///
/// \brief Abstract shape class
///
class Shape : public Widget
{
protected:
    ///
    /// \brief Constructor
    ///
    Shape(std::string name, float x, float y, Color color, bool filled)
        : Widget(name, x, y)
    {
        m_color  = color;
        m_filled = filled;
        m_type   = Widget::Type::Shape;
    }

    Shape() = delete;
    Color m_color;
    bool  m_filled;
};

///
/// \class Circle
///
/// \brief Circle widget
///
class Circle : public Shape
{
public:
    ///
    /// \brief Protected constructor
    ///
    Circle(std::string name,
           float       x,
           float       y,
           float       radius,
           Color       color,
           bool        filled = false)
        : Shape(name, x, y, color, filled)
    {
        m_radius = radius;
    }

    ///
    /// \brief Render
    ///
    virtual void render(const bool inWindow)
    {
        ImDrawList* drawList;

        if (inWindow)
        {
            drawList = ImGui::GetWindowDrawList();
        }
        else
        {
            drawList = ImGui::GetOverlayDrawList();
        }

        if (m_filled)
        {
            drawList->AddCircleFilled(ImVec2(m_position[0], m_position[1]),
                m_radius,
                GUIOverlay::Utilities::convertToGUIColor(m_color),
                24);
        }
        else
        {
            drawList->AddCircle(ImVec2(m_position[0], m_position[1]),
                m_radius,
                GUIOverlay::Utilities::convertToGUIColor(m_color),
                24);
        }
    }

    ///
    /// \brief Set radius
    ///
    void setRadius(const float radius)
    {
        m_radius = radius;
    }

    ///
    /// \brief Get radius
    ///
    const float getRadius()
    {
        return m_radius;
    }

protected:
    Circle() = delete;
    float m_radius;
};

///
/// \class Rectangle
///
/// \brief Rectangle widget
///
class Rectangle : public Shape
{
public:
    ///
    /// \brief Protected constructor
    ///
    Rectangle(std::string name,
              float       x,
              float       y,
              float       width,
              float       height,
              Color       color,
              bool        filled = false)
        : Shape(name, x, y, color, filled)
    {
        m_height = height;
        m_width  = width;
    }

    ///
    /// \brief Render
    ///
    virtual void render(const bool inWindow)
    {
        ImDrawList* drawList;

        if (inWindow)
        {
            drawList = ImGui::GetWindowDrawList();
        }
        else
        {
            drawList = ImGui::GetOverlayDrawList();
        }

        if (m_filled)
        {
            drawList->AddRectFilled(ImVec2(m_position[0], m_position[1]),
                ImVec2(m_width + m_position[0], m_height + m_position[1]),
                GUIOverlay::Utilities::convertToGUIColor(m_color),
                0);
        }
        else
        {
            drawList->AddRect(ImVec2(m_position[0], m_position[1]),
                ImVec2(m_width + m_position[0], m_height + m_position[1]),
                GUIOverlay::Utilities::convertToGUIColor(m_color),
                0);
        }
    }

    ///
    /// \brief Set width
    ///
    void setWidth(const float width)
    {
        m_width = width;
    }

    ///
    /// \brief Set height
    ///
    void setHeight(const float height)
    {
        m_height = height;
    }

    ///
    /// \brief Get width
    ///
    const float getWidth()
    {
        return m_width;
    }

    ///
    /// \brief Get height
    ///
    const float getHeight()
    {
        return m_height;
    }

protected:
    Rectangle() = delete;
    float m_height;
    float m_width;
};
}
} // imstk
