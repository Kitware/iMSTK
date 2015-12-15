// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Mesh/MeshMapAffine.h"

#include <memory>

MeshMapAffine::MeshMapAffine(const TransformType& M)
{
    this->affineTransform = M;
}

MeshMapAffine::MeshMapAffine(std::shared_ptr<Core::BaseMesh>& masterMesh,
                             std::shared_ptr<Core::BaseMesh>& slaveMesh,
                             const TransformType& M)
    : MeshMap(masterMesh,slaveMesh)

{
    this->affineTransform = M;
}

MeshMapAffine::~MeshMapAffine()
{
}

void MeshMapAffine::apply()
{
    // First copy the co-ordinates of the nodal points of master mesh to slave mesh
    this->slaveMesh->setVertices(this->masterMesh->getVertices());

    // Do the affine transform
    this->slaveMesh->transform(this->affineTransform);
}

void MeshMapAffine::compute()
{
    // do nothing here since it is an identity map
}

const MeshMapAffine::TransformType& MeshMapAffine::getAffineTransform() const
{
    return this->affineTransform;
}
