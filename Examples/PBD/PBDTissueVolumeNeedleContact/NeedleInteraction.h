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

#include "imstkMacros.h"
#include "imstkPbdRigidObjectCollision.h"

using namespace imstk;

class NeedleEmbeddedCH;
class NeedleObject;

namespace imstk
{
class PbdObject;
class TetraToLineMeshCD;
} // namespace imstk

///
/// \class NeedleInteraction
///
/// \brief Defines interaction between NeedleObject and PbdObject
///
class NeedleInteraction : public PbdRigidObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<PbdObject>    tissueObj,
                      std::shared_ptr<NeedleObject> needleObj);
    ~NeedleInteraction() override = default;

    IMSTK_TYPE_NAME(NeedleInteraction)

    std::shared_ptr<TetraToLineMeshCD> getEmbeddingCD() const { return tetMeshCD; }
    std::shared_ptr<NeedleEmbeddedCH> getEmbeddingCH() const { return embeddedCH; }

    ///
    /// \brief Setup connectivity of task graph
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<TetraToLineMeshCD> tetMeshCD;
    std::shared_ptr<NeedleEmbeddedCH>  embeddedCH;

    std::shared_ptr<TaskNode> embeddingCDNode = nullptr;
    std::shared_ptr<TaskNode> embeddingCHNode = nullptr;
};