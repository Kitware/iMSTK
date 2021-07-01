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

#include "imstkCollisionData.h"
#include "imstkDebugGeometryObject.h"

namespace imstk
{
///
/// \class CollisionDataDebugObject
///
/// \brief Class for visualizing CollisionData. Give it collision data and add it to the scene
/// This class intentionally does not automatically update it provides debugUpdate which may be
/// called by the user when appropriate (for debug purposes)
///
class CollisionDataDebugObject : public DebugGeometryObject
{
public:
    CollisionDataDebugObject() : DebugGeometryObject() { }
    virtual ~CollisionDataDebugObject() = default;

public:
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

    void setInputCD(std::shared_ptr<CollisionData> cdData) { m_cdData = cdData; }
    std::shared_ptr<CollisionData> getInputCD() const { return m_cdData; }

    void setPrintContacts(const bool printContacts) { m_printContacts = printContacts; }

protected:
    std::shared_ptr<CollisionData> m_cdData = nullptr;
    bool m_printContacts = false;
};
}