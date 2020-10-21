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

#include "imstkScreenCaptureUtility.h"

namespace imstk
{
unsigned int
ScreenCaptureUtility::getScreenShotNumber() const
{
    return m_screenShotNumber;
}

void
ScreenCaptureUtility::setScreenShotPrefix(const std::string& newPrefix)
{
    if (m_screenShotPrefix.compare(newPrefix) != 0)
    {
        m_screenShotPrefix = newPrefix;
        m_screenShotNumber = 0;
    }
}

void
ScreenCaptureUtility::resetScreenShotNumber()
{
    m_screenShotNumber = 0;
}
} // imstk
