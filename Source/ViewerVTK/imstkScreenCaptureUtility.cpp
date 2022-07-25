/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkScreenCaptureUtility.h"

namespace imstk
{
ScreenCaptureUtility::ScreenCaptureUtility(std::string prefix) :
    m_screenShotNumber(0),
    m_screenShotPrefix(prefix)
{
}

std::string
ScreenCaptureUtility::saveScreenShot()
{
    std::string captureName = m_screenShotPrefix + std::to_string(m_screenShotNumber);
    ++m_screenShotNumber;
    return saveScreenShot(captureName);
}

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
} // namespace imstk