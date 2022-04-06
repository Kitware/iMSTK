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

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkMacros.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class SurfaceMeshToSurfaceMeshCD
///
/// \brief Collision detection for surface meshes
///
class SurfaceMeshToSurfaceMeshCD : public CollisionDetectionAlgorithm
{
public:
    SurfaceMeshToSurfaceMeshCD();
    virtual ~SurfaceMeshToSurfaceMeshCD() override = default;

    IMSTK_TYPE_NAME(SurfaceMeshToSurfaceMeshCD)

public:
    void setMaxNumContacts(const int maxNumContacts) { m_maxNumContacts = maxNumContacts; }
    const int getMaxNumContacts() const { return m_maxNumContacts; }

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    virtual void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;

protected:
    std::vector<std::pair<int, int>> m_intersectingPairs;
    int m_maxNumContacts = 1000;
};
} // namespace imstk