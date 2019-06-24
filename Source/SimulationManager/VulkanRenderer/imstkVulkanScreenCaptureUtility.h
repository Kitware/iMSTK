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

#include "imstkScreenCaptureUtility.h"

#include <string>

namespace imstk
{
///
/// \class VulkanScreenCaptureUtility
///
/// \brief Utility class to manage screen capture through Vulkan
///
class VulkanScreenCaptureUtility : public ScreenCaptureUtility
{
public:
    ///
    /// \brief Constructor
    ///
    VulkanScreenCaptureUtility(const std::string prefix = "Screenshot-");

    ///
    /// \brief Destructor
    ///
    ~VulkanScreenCaptureUtility() = default;

    ///
    /// \brief Saves the screenshot as a png file
    ///
    virtual void saveScreenShot();

protected:
};
} // imstk
