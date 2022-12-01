/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVisualTestingUtils.h"

namespace imstk
{
class Camera;
class CollisionDataDebugModel;
class CollisionDetectionAlgorithm;
class Entity;
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
    std::shared_ptr<imstk::Entity> m_cdObj2 = nullptr;
    std::shared_ptr<imstk::Entity> m_cdObj1 = nullptr;
    std::shared_ptr<imstk::Geometry> m_cdGeom1       = nullptr;
    std::shared_ptr<imstk::Geometry> m_cdGeom2       = nullptr;

    std::shared_ptr<imstk::Camera> m_camera = nullptr;

    std::shared_ptr<imstk::CollisionDetectionAlgorithm> m_collisionMethod = nullptr;
    std::shared_ptr<imstk::CollisionDataDebugModel>     m_cdDebugObject   = nullptr;

    bool m_geom1Moveable = false;
    bool m_geom2Moveable = true;
    bool m_printContacts = false;
};