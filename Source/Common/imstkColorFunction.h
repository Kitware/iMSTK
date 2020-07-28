/*======================================================================

Library: iMSTK

Copyright(c) Kitware, Inc.& Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.

======================================================================== = */

#pragma once

#include "imstkColor.h"

#include <vector>
#include <algorithm>

namespace imstk
{
///
/// \class ColorFunction
///
/// \brief A regularly/structured table of colors to lookup by value
/// \todo: Currently just serves as storage, write the mapping functions
///
class ColorFunction
{
public:
    enum class ColorSpace
    {
        RGB,
        DIVERING,
        HSV,
        LAB
    };

public:
    ColorFunction() = default;
    virtual ~ColorFunction() = default;

public:
    int getNumberOfColors() const { return static_cast<int>(m_table.size()); }
    const Color& getColor(int i) const { return m_table[i]; }
    const Vec2d& getRange() const { return m_range; }
    const ColorSpace& getColorSpace() const { return m_colorSpace; }

    void setNumberOfColors(int numColors) { m_table.resize(numColors); }
    void setColor(int i, const Color& color) { m_table[i] = color; }
    void setRange(double min, double max) { setRange(Vec2d(min, max)); }
    void setRange(const Vec2d& range) { this->m_range = range; }
    void setColorSpace(const ColorSpace& space) { this->m_colorSpace = space; }

    ///
    /// \brief Clears all colors from the lookup table
    ///
    void clear() { m_table.clear(); }

protected:
    std::vector<Color> m_table;
    Vec2d      m_range;
    ColorSpace m_colorSpace;
};
}