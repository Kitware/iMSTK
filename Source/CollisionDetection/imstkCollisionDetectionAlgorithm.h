/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionData.h"
#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class Geometry;

///
/// \class CollisionDetectionAlgorithm
///
/// \brief Base class for all collision detection classes. CollisionDetection
/// classes produce CollisionData between two geometries A and B. CollisionData
/// has two sides. That is, the contact information to resolve collision for
/// geometry A and the contact info to resolve geometry B.
///
/// Subclassed algorithms may produce A, B, or both. To implement, one should
/// implement computeCollisionDataAB, or computeCollisionDataA and
/// computeCollisionDataB. If A or B is not implemented, AB will be called. If AB
/// is not implemented, A and B will be called.
///
/// CollisionDetection::setGenerateCD(bool, bool) can be used to request sides.
/// CD subclasses can provide defaults for this as well and not expect the user
/// to touch it.
///
class CollisionDetectionAlgorithm : public GeometryAlgorithm
{
protected:
    CollisionDetectionAlgorithm();

public:
    virtual ~CollisionDetectionAlgorithm() = default;

    ///
    /// \brief Returns collision detection type string name
    ///
    virtual const std::string getTypeName() const = 0;

    ///
    /// \brief Returns output collision data
    ///
    inline const std::shared_ptr<CollisionData> getCollisionData() const { return (*m_collisionDataVector)[0]; }

    inline const std::shared_ptr<CollisionData> getCollisionData(int i) const
    {
        return m_collisionDataVector->size() > i ? m_collisionDataVector->at(i) : nullptr;
    }

    inline const std::shared_ptr<std::vector<std::shared_ptr<CollisionData>>>& getCollisionDataVector() const { return m_collisionDataVector; }

    inline size_t getCollisionDataVectorSize() const
    {
        return m_collisionDataVector->size();
    }

    ///
    /// \brief If generateA is false, CD data will not be generated for input0,A
    /// Similarly, if generateB is false, CD data will not be generated for input1,B
    ///
    void setGenerateCD(const bool generateA, const bool generateB)
    {
        m_generateCD_A = generateA;
        m_generateCD_B = generateB;
    }

    void setInputGeometryA(std::shared_ptr<Geometry> geometryA) { setInput(geometryA, 0); }

    void setInputGeometryB(std::shared_ptr<Geometry> geometryB) { setInput(geometryB, 1); }

protected:
    ///
    /// \brief Check inputs are correct (always works reversibly)
    /// \return true if all inputs match the requirements, false if not
    ///
    virtual bool areInputsValid() override;

    ///
    /// \brief Compute the collision data
    ///
    virtual void requestUpdate() override;

    ///
    /// \brief Compute collision data for both sides at once, default implementation
    /// just calls computeCollisionDataA and computeCollisionDataB
    ///
    virtual void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB)
    {
        computeCollisionDataA(geomA, geomB, elementsA);
        computeCollisionDataB(geomA, geomB, elementsB);
    }

    ///
    /// \brief Compute collision data for side A (implement as if flip=true)
    ///
    virtual void computeCollisionDataA(
        std::shared_ptr<Geometry>      imstkNotUsed(geomA),
        std::shared_ptr<Geometry>      imstkNotUsed(geomB),
        std::vector<CollisionElement>& imstkNotUsed(elementsA)) { m_computeColDataAImplemented = false; }

    ///
    /// \brief Compute collision data for side B (implement as if flip=true)
    ///
    virtual void computeCollisionDataB(
        std::shared_ptr<Geometry>      imstkNotUsed(geomA),
        std::shared_ptr<Geometry>      imstkNotUsed(geomB),
        std::vector<CollisionElement>& imstkNotUsed(elementsB)) { m_computeColDataBImplemented = false; }

    std::shared_ptr<std::vector<std::shared_ptr<CollisionData>>> m_collisionDataVector;

    bool m_flipOutput   = false;
    bool m_generateCD_A = true;
    bool m_generateCD_B = true;

    bool m_computeColDataAImplemented = true;
    bool m_computeColDataBImplemented = true;
};
} // namespace imstk