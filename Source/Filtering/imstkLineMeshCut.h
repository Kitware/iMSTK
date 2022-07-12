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

#include "imstkMeshCut.h"

namespace imstk
{
class ImplicitGeometry;
class LineMesh;
class SurfaceMesh;

enum class SegmentCutType
{
    NONE = 0,
    EDGE = 1 // Edge split
};

///
/// \class LineMeshCut
///
/// \brief This filter cuts the lines of a LineMesh into smaller
/// lines using input cutting geometry
/// Only supports convex shaped cutting, ie: An edge can't be
/// split twice
///
class LineMeshCut : public MeshCut
{
public:
    LineMeshCut();
    ~LineMeshCut() override = default;

    std::shared_ptr<LineMesh> getOutputMesh();
    void setInputMesh(std::shared_ptr<LineMesh> mesh);

protected:
    void refinement(std::shared_ptr<AbstractCellMesh> outputGeom,
                    std::map<int, bool>& cutVerts) override;
    void splitVerts(std::shared_ptr<AbstractCellMesh> outputGeom,
                    std::map<int, bool>& cutVerts,
                    std::shared_ptr<Geometry> cuttingGeom) override;

    std::shared_ptr<std::vector<CutData>> generateCutData(
        std::shared_ptr<Geometry>         cuttingGeom,
        std::shared_ptr<AbstractCellMesh> geomToCut) override;

    std::shared_ptr<std::vector<CutData>> generateImplicitCutData(
        std::shared_ptr<ImplicitGeometry> cuttingGeom,
        std::shared_ptr<LineMesh>         geomToCut);

    std::shared_ptr<std::vector<CutData>> generateSurfaceMeshCutData(
        std::shared_ptr<SurfaceMesh> cuttingGeom,
        std::shared_ptr<LineMesh>    geomToCut);
};
} // namespace imstk