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

#include "Assembler/Assembler.h"

Assembler::Assembler()
{
}

Assembler::Assembler(std::shared_ptr<CollisionContext>& collContext)
{
    this->collisionContext = collContext;
    initiateSystemOfEquations();
}

Assembler::~Assembler()
{
}

void Assembler::consolidateType1Interactions()
{
    auto ch = this->collisionContext->getContactHandlers();

    for (auto it = ch.begin(); it != ch.end(); ++it)
    {
        if ((*it)->getType() == core::ContactHandlingType::Penalty)
        {
            (*it)->getFirstSceneObject()->setCumulativeContactForces(
                std::static_cast<std::shared_ptr<PenaltyContactHandling>>
                (*it)->getContactForce());
        }
    }
}

void Assembler::initiateSystemOfEquations()
{
    int dofSize;
    auto islands = this->collisionContext->getIslands();

    for (auto i = islands.begin(); i != islands.end(); ++i)
    {
        dofSize = 0;
        for (auto j = i->begin(); j != i->end(); ++j)
        {
            dofSize += this->collisionContext->getObjectWithIndex(*j)->getNumDof();
        }
    }
}

void Assembler::updateSubsystems()
{

}

void Assembler::stackSparseMatrices(SparseMatrixd& parent, SparseMatrixd& addOn)
{
    int expandedSize = parent.size() + addOn.size();
    parent.resize(expandedSize, expandedSize);



    for (int i = 0; i < )

}