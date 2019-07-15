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

namespace imstk
{
namespace GUIOverlay
{
///
/// \class GUIUtilities
///
/// \brief Utility functions for GUI management
///
class Utilities
{
public:
    ///
    /// \brief Initializes the GUI system with a theme
    ///
    static void initializeGUISystem()
    {
        auto io = ImGui::GetIO();
        io.IniFilename = nullptr; // resets the GUI layout on application restart
    }

    static ImU32 convertToGUIColor(const Color& color)
    {
        return ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, color.a));
    }
};      // GUIUtilities
} // GUI
} // imstk
