/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSceneObject.h"
#include "imstkCellMesh.h"
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
/// \class PbdObjectCutting
///
/// \brief This class defines a cutting interaction between a PbdObject and
/// a CollidingObject. PbdObjectCutting::apply can be used to perform a discrete
/// cut given the current states of both objects.
///
class PbdObjectCutting : public SceneObject
{
public:
    PbdObjectCutting(std::shared_ptr<PbdObject> pbdObj, std::shared_ptr<CollidingObject> cutObj);
    ~PbdObjectCutting() override = default;

    IMSTK_TYPE_NAME(PbdObjectCutting)

    ///
    /// \brief Epsilon controls the distance a point needs to be to
    /// be considered "inside" the cutting zone
    /// @{
    double getEpsilon() const { return m_epsilon; }
    void setEpsilon(const double eps) { m_epsilon = eps; }
    /// @}

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
    template<int N>
    void addCells(std::shared_ptr<CellMesh<N>> pbdMesh,
                  std::shared_ptr<VecDataArray<int, N>> newCells)
    {
        std::shared_ptr<VecDataArray<int, N>> cells     = pbdMesh->getCells();
        const int                             nCells    = cells->size();
        const int                             nNewCells = newCells->size();

        cells->reserve(nCells + nNewCells);
        for (int i = 0; i < nNewCells; i++)
        {
            const Vec3i& cell = (*newCells)[i];
            cells->push_back(cell);
            for (int j = 0; j < N; j++)
            {
                m_addConstraintVertices->insert(cell[j]);
            }
        }
    }

    ///
    /// \brief Modify existing elements of pbdObj
    ///
    void modifyTriangles(std::shared_ptr<SurfaceMesh> pbdMesh,
                         std::shared_ptr<std::vector<size_t>> elementIndices,
                         std::shared_ptr<VecDataArray<int, 3>> elements);

    double m_epsilon = 0.1;

    std::shared_ptr<PbdObject>       m_objA = nullptr;
    std::shared_ptr<CollidingObject> m_objB = nullptr;

    std::shared_ptr<std::unordered_set<size_t>> m_removeConstraintVertices = std::make_shared<std::unordered_set<size_t>>();
    std::shared_ptr<std::unordered_set<size_t>> m_addConstraintVertices    = std::make_shared<std::unordered_set<size_t>>();
};
} // namespace imstk