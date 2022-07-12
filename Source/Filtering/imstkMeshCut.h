/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc.

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

#include "imstkGeometryAlgorithm.h"
#include "imstkMath.h"

#include <map>
#include <unordered_set>
#include <set>

namespace imstk
{
class AbstractCellMesh;
class AnalyticalGeometry;
class Geometry;
class SurfaceMesh;
template<typename T, int N> class VecDataArray;

struct CutData
{
    public:
        Vec3d cutCoords[2];
        Vec3d initCoords[2];
        int cellId   = -1; // Id of the cell (line, triangle, ...)
        int ptIds[2] = { -1, -1 };
        int cutType  = 0;
};

///
/// \class MeshCut
///
/// \brief Base abstract class for discrete cut algorithms
/// \todo: Generalize the cutting into composited bits and provide one concrete MeshCut
/// instead of many subclasses of this
///
class MeshCut : public GeometryAlgorithm
{
protected:
    MeshCut();

public:
    ~MeshCut() override = default;

    void requestUpdate() override;

    std::shared_ptr<std::map<int, int>> getCutVertMap() { return m_CutVertMap; }

    imstkGetMacro(CutData, std::shared_ptr<std::vector<CutData>>);
    imstkSetMacro(CutData, std::shared_ptr<std::vector<CutData>>);

    imstkGetMacro(CutGeometry, std::shared_ptr<Geometry>);
    imstkSetMacro(CutGeometry, std::shared_ptr<Geometry>);

    imstkGetMacro(Epsilon, double);
    imstkSetMacro(Epsilon, double);

    imstkGetMacro(RemoveConstraintVertices, std::shared_ptr<std::unordered_set<size_t>>);
    imstkGetMacro(AddConstraintVertices, std::shared_ptr<std::unordered_set<size_t>>);

protected:
    ///
    /// \brief Generate CutData which defines how the cut should be performed
    ///
    virtual std::shared_ptr<std::vector<CutData>> generateCutData(
        std::shared_ptr<Geometry>         cuttingGeom,
        std::shared_ptr<AbstractCellMesh> geomToCut) = 0;

    ///
    /// \brief Refine the mesh adding vertices and changing connectivity
    /// along the cut
    /// \param Geometry to cut
    /// \param vertices on cutting path and whether they should be split
    ///
    virtual void refinement(
        std::shared_ptr<AbstractCellMesh> outputGeom,
        std::map<int, bool>& cutVerts) = 0;

    ///
    /// \brief Split the cutting vertices, separating them into two
    /// \param Geometry to cut
    /// \param vertices on cutting path and whether they should be split
    /// \param Geometry to cut with
    ///
    virtual void splitVerts(
        std::shared_ptr<AbstractCellMesh> outputGeom,
        std::map<int, bool>& cutVerts,
        std::shared_ptr<Geometry> cuttingGeom) = 0;

    ///
    /// \brief Determine the sign of the point
    /// -1 if inside, 1 if outside, 0 if on boundary defined by epsilon
    ///
    int ptBoundarySign(const Vec3d& pt, std::shared_ptr<Geometry> geometry);

    template<int N>
    bool vertexOnBoundary(std::shared_ptr<VecDataArray<int, N>> cells,
                          std::set<int>& cellSet)
    {
        std::set<int> nonRepeatNeighborVerts;
        for (const auto& cellId : cellSet)
        {
            for (int i = 0; i < N; i++)
            {
                const int ptId = (*cells)[cellId][i];
                if (nonRepeatNeighborVerts.find(ptId) != nonRepeatNeighborVerts.end())
                {
                    nonRepeatNeighborVerts.erase(ptId);
                }
                else
                {
                    nonRepeatNeighborVerts.insert(ptId);
                }
            }
        }
        return (nonRepeatNeighborVerts.size() >= 2);
    }

    bool pointProjectionInSurface(const Vec3d& pt, std::shared_ptr<SurfaceMesh> surface);

    std::shared_ptr<std::vector<CutData>> m_CutData = nullptr;

    std::shared_ptr<std::map<int, int>> m_CutVertMap =
        std::make_shared<std::map<int, int>>();

    std::shared_ptr<Geometry> m_CutGeometry = nullptr;

    std::shared_ptr<std::unordered_set<size_t>> m_RemoveConstraintVertices = nullptr;
    std::shared_ptr<std::unordered_set<size_t>> m_AddConstraintVertices    = nullptr;

    double m_Epsilon = 1.0;
};
} // namespace imstk