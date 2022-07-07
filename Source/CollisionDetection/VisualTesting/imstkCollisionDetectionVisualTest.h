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

#include "imstkVisualTestingUtils.h"

namespace imstk
{
class Camera;
class CollisionDataDebugObject;
class CollisionDetectionAlgorithm;
class CollidingObject;
class Geometry;
} // namespace imstk

///
/// \class CollisionDetectionVisualTest
///
/// \brief Test that involves two collision geometries and a collision method
///
class CollisionDetectionVisualTest : public VisualTest
{
public:
    void SetUp() override;

    ///
    /// \brief Create a scene composed of two collision objects with
    /// the respective collision geometries and method
    ///
    void createScene();

    ///
    /// \brief Adds the default scheme for key controls
    /// \param Key code
    /// \param Geometry to move
    /// \return Whether or not geom was moved
    ///
    bool moveGeometryByKey(const unsigned char              key,
                           std::shared_ptr<imstk::Geometry> geom);

public:
    std::shared_ptr<imstk::CollidingObject> m_cdObj2 = nullptr;
    std::shared_ptr<imstk::CollidingObject> m_cdObj1 = nullptr;
    std::shared_ptr<imstk::Geometry> m_cdGeom1       = nullptr;
    std::shared_ptr<imstk::Geometry> m_cdGeom2       = nullptr;

    std::shared_ptr<imstk::Camera> m_camera = nullptr;

    std::shared_ptr<imstk::CollisionDetectionAlgorithm> m_collisionMethod = nullptr;
    std::shared_ptr<imstk::CollisionDataDebugObject>    m_cdDebugObject   = nullptr;

    bool m_geom1Moveable = false;
    bool m_geom2Moveable = true;
    bool m_printContacts = false;
};