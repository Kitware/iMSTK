/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionInteraction.h"
#include "imstkMacros.h"

#include <unordered_set>
#include <vector>

namespace imstk
{
template<typename T, int N> class VecDataArray;
class CollidingObject;
class PbdObject;
class SurfaceMesh;

///
/// \class PbdObjectCuttingPair
///
/// \brief This class defines a cutting interaction between a PbdObject and a CollidingObject
/// call apply to perform the cut given the current states of both objects. A discrete cut is
/// performed, not for calling continuously.
///
class PbdObjectCutting : public SceneObject
{
public:
    PbdObjectCutting(std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> cutObj);
    ~PbdObjectCutting() override = default;

    IMSTK_TYPE_NAME(PbdObjectCutting)

    ///
    /// \brief Applies the cut when called
    ///
    void apply();

protected:
    ///
    /// \brief Add new vertices to pbdObj
    ///
    void addVertices(std::shared_ptr<SurfaceMesh> pbdMesh,
                     std::shared_ptr<VecDataArray<double, 3>> vertices,
                     std::shared_ptr<VecDataArray<double, 3>> initialVertices);

    ///
    /// \brief Modify current vertices of pbdObj
    ///
    void modifyVertices(std::shared_ptr<SurfaceMesh> pbdMesh,
                        std::shared_ptr<std::vector<size_t>> vertexIndices,
                        std::shared_ptr<VecDataArray<double, 3>> vertices,
                        std::shared_ptr<VecDataArray<double, 3>> initialVertices);

    ///
    /// \brief Add new elements to pbdObj
    ///
    void addTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                      std::shared_ptr<VecDataArray<int, 3>> elements);

    ///
    /// \brief Modify existing elements of pbdObj
    ///
    void modifyTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                         std::shared_ptr<std::vector<size_t>> elementIndices,
                         std::shared_ptr<VecDataArray<int, 3>> elements);

    std::shared_ptr<PbdObject>       m_objA = nullptr;
    std::shared_ptr<CollidingObject> m_objB = nullptr;

    std::shared_ptr<std::unordered_set<size_t>> m_removeConstraintVertices = std::make_shared<std::unordered_set<size_t>>();
    std::shared_ptr<std::unordered_set<size_t>> m_addConstraintVertices    = std::make_shared<std::unordered_set<size_t>>();
};
} // namespace imstk