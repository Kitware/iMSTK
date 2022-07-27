/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <string>

namespace imstk
{
///
/// \class ScreenCaptureUtility
///
/// \brief Utility class to manage screen capture
///
class ScreenCaptureUtility
{
public:
    ///
    /// \brief Constructor
    ///
    ScreenCaptureUtility(std::string prefix = "Screenshot-");

    ///
    /// \brief Saves a screenshot with a name of <prefix><screenshotNumber>.<implementationImageType>
    ///        the <ImplementationImageType> is most likely `.png`.
    /// \return the file name that was actually used to store the file, "" if the file storage failed
    std::string saveScreenShot();

    ///
    /// \brief Saves a screenshot with the given name, the implementation will add the image
    ///        type used to store the file
    /// \param name base name for the file to store
    /// \return the file name that was actually used to store the file, "" if the file storage failed
    virtual std::string saveScreenShot(const std::string& name) = 0;

    ///
    /// \brief Returns the number of the next screenshot
    ///
    unsigned int getScreenShotNumber() const;

    ///
    /// \brief set/reset the prefix and the count numbers
    ///
    void setScreenShotPrefix(const std::string& newPrefix);

    ///
    /// \brief reset the screenshot number
    ///
    void resetScreenShotNumber();

protected:
    virtual ~ScreenCaptureUtility() = default;

    unsigned int m_screenShotNumber = 0; ///< screen shot number is added to the file prefix, and incremented everytime a screen shot is taken
    std::string  m_screenShotPrefix;     ///< the prefix for the screenshots to be saved
};
} // namespace imstk