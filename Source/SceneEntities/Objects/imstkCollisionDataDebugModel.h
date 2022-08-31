/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionData.h"
#include "imstkDebugGeometryModel.h"

namespace imstk
{
///
/// \class CollisionDataDebugModel
///
/// \brief Class for visualizing CollisionData. Give it collision data and add it to the scene
/// This class intentionally does not automatically update it provides debugUpdate which may be
/// called by the user when appropriate (for debug purposes)
///
class CollisionDataDebugModel : public DebugGeometryModel
{
public:
    CollisionDataDebugModel() : DebugGeometryModel() { }
    ~CollisionDataDebugModel() override = default;

    ///
    /// \brief Append visual representation to the debug meshes for the given contact and geometry
    ///
    void addCollisionElement(std::shared_ptr<PointSet> pointSet, const CollisionElement& elem);

    ///
    /// \brief Print the contact information
    ///
    void printContactInfo(const CollisionElement& elem);

    ///
    /// \brief Updates visual representation to the *current* cd (we don't use the built in
    /// update as its useful to call this in specific spots in code for debug purposes)
    ///
    void debugUpdate();

    ///
    /// \brief Set/Get the input collision data for which geometry is generated for
    ///@{
    void setInputCD(std::shared_ptr<CollisionData> cdData) { m_cdData = cdData; }
    std::shared_ptr<CollisionData> getInputCD() const { return m_cdData; }
    ///@}

    ///
    /// \brief Print collision contact data to stdout
    ///@{
    void setPrintContacts(const bool printContacts) { m_printContacts = printContacts; }
    bool getPrintContacts() const { return m_printContacts; }
    ///@}

    ///
    /// \brief Set the amount of simulation frames to cache data for, this is useful if you have
    /// numerous simulation frames per render and data doesn't persist from one to the next
    ///@{
    void setClearRate(const int clearRate) { m_clearRate = clearRate; }
    int getClearRate() const { return m_clearRate; }
    ///@}

    ///
    /// \brief If on, frames without collision data won't be counted for clear rate
    ///@{
    void setCountEmptyFrames(const bool countEmptyFrames) { m_countEmptyFrames = countEmptyFrames; }
    int getCountEmptyFrames() const { return m_countEmptyFrames; }
///@}

protected:
    std::shared_ptr<CollisionData> m_cdData = nullptr;
    bool m_printContacts    = false;
    int  m_clearRate        = 1;
    int  m_frameCounter     = 0;
    bool m_countEmptyFrames = true;
};
} // namespace imstk