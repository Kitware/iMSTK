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

#include "g3log/g3log.hpp"

namespace imstk
{
class VulkanValidation
{
public:
    static char* getValidationLayer()
    {
        return "VK_LAYER_LUNARG_standard_validation";
    }

    static char* getValidationExtension()
    {
        return "VK_EXT_debug_report";
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT      debugReportFlags,
                                                              VkDebugReportObjectTypeEXT debugReportObjectType,
                                                              uint64_t                   callbackObject,
                                                              size_t                     level,
                                                              int32_t                    code,
                                                              const char*                prefix,
                                                              const char*                message,
                                                              void*                      data)
    {
        LOG(WARNING) << prefix << ": " << message;
        return VK_FALSE;
    }
};
}
