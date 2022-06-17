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