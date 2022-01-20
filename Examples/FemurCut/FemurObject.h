///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#pragma once

#include "imstkLevelSetDeformableObject.h"

#include <unordered_set>

namespace imstk
{
class LocalMarchingCubes;
class TaskNode;
} // namespace imstk

using namespace imstk;

///
/// \class FemurObject
///
/// \brief The FemurObject implements a localized marching cubes. That is we've split
/// up an image into a bunch of chunks. Our model reports on which voxels of the image
/// were modified, we supply this to the local marching cubes and it will update
/// only the respective chunks.
///
class FemurObject : public LevelSetDeformableObject
{
public:
    FemurObject();
    ~FemurObject() override = default;

public:
    ///
    /// \brief Update the isosurface before rendering, the isosurface
    /// is not used for simulation so we can afford to update it
    /// less frequently
    ///
    void visualUpdate() override;

    ///
    /// \brief Creates visual models for any chunk that has non-zero vertices
    /// and is not already generated
    ///
    void createVisualModels();

    void setUseRandomChunkColors(const bool useRandom) { m_useRandomChunkColors = useRandom; }
    bool getUseRandomChunkColors() const { return m_useRandomChunkColors; }

protected:
    ///
    /// \brief Forwards/copies the levelsets list of modified voxels to the isosurface
    /// extraction filters list of modified voxels
    ///
    void updateModifiedVoxels();

    ///
    /// \brief Setup connectivity of task graph
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<LocalMarchingCubes> m_isoExtract;
    std::unordered_set<int>   m_chunksGenerated;               // Lazy generation of chunks
    std::shared_ptr<TaskNode> m_forwardModifiedVoxels;
    bool m_useRandomChunkColors = false;
};