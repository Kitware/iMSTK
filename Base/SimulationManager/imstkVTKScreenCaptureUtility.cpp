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

#include "imstkVTKScreenCaptureUtility.h"

#include "g3log/g3log.hpp"

namespace imstk
{
VTKScreenCaptureUtility::VTKScreenCaptureUtility(vtkRenderWindow* const rw, const std::string prefix /*= "Screenshot-"*/)
    : m_screenShotNumber(0)
{
    m_screenShotPrefix = prefix;
    if (rw != nullptr)
    {
        m_renderWindow = rw;
    }
}


void
VTKScreenCaptureUtility::saveScreenShot()
{
    if (m_renderWindow == nullptr)
    {
        LOG(WARNING) << "Render window has not been set yet! ";
        return;
    }


    if (m_windowToImageFilter->GetInput() == nullptr)
    {
        m_windowToImageFilter->SetInput(m_renderWindow);

        m_windowToImageFilter->SetMagnification(1);
        m_windowToImageFilter->SetInputBufferTypeToRGB();
        m_windowToImageFilter->ReadFrontBufferOff();
        m_windowToImageFilter->Update();

        m_pngWriter->SetInputConnection(m_windowToImageFilter->GetOutputPort());
    }

    m_windowToImageFilter->Modified();

    std::string captureName = m_screenShotPrefix + std::to_string(m_screenShotNumber) + ".png";

    m_pngWriter->SetFileName(captureName.data());
    m_pngWriter->Write();

    LOG(INFO) << "Screen shot " << m_screenShotNumber << " saved as " << captureName << "\n";

    m_screenShotNumber++;
}


unsigned int
VTKScreenCaptureUtility::getScreenShotNumber() const
{
    return m_screenShotNumber;
}

void
VTKScreenCaptureUtility::setScreenShotPrefix(const std::string newPrefix)
{
    if (m_screenShotPrefix.compare(newPrefix) != 0)
    {
        m_screenShotPrefix = newPrefix;
        m_screenShotNumber = 0;
    }
}

void
VTKScreenCaptureUtility::resetScreenShotNumber()
{
    m_screenShotNumber = 0;
}
} // imstk
