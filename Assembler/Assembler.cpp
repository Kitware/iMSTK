// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#include "Assembler/Assembler.h"
#include "Core/ContactHandling.h"
#include "Core/Model.h"
#include "TimeIntegrators/OdeSystem.h"

Assembler::Assembler(std::shared_ptr<CollisionContext> collContext)
{
    this->collisionContext = collContext;
    this->initSystem();
}

//---------------------------------------------------------------------------
void Assembler::type1Interactions()
{
    if(!this->collisionContext)
    {
        // TODO: Log this
        return;
    }

    auto ch = this->collisionContext->getContactHandlers();

    for(auto &ch : this->collisionContext->getContactHandlers())
    {
        const auto &forces = ch->getContactForces();
        auto sceneModel = ch->getSecondSceneObject();
        sceneModel->updateExternalForces(forces);
    }
}

//---------------------------------------------------------------------------
void Assembler::initSystem()
{
    for(auto &rows : this->collisionContext->getIslands())
    {
        size_t dofSize = 0;
        size_t nnz = 0;
        std::vector<const core::SparseMatrixd*> systemMatrices;
        std::vector<const core::Vectord*> rhsVector;
        for(auto &col : rows)
        {
            auto sceneModel = this->collisionContext->getSceneModel(col);
            auto odeSystem = std::dynamic_pointer_cast<OdeSystem>(
                    this->collisionContext->getSceneModel(col));
            if(sceneModel && odeSystem)
            {
                dofSize += sceneModel->getNumOfDOF();
                systemMatrices.push_back(&odeSystem->getSystemMatrix());
                rhsVector.push_back(&odeSystem->getRHS());
                nnz += systemMatrices.back()->nonZeros();
            }
        }

        if(dofSize > 0)
        {
            this->A.emplace_back(dofSize,dofSize);
            this->A.back().reserve(nnz);
            this->b.emplace_back(dofSize);
            auto system = std::make_shared<LinearSystemType>(this->A.back(),this->b.back());

            this->systemOfEquationsList.push_back(system);

            size_t size = 0;
            for(size_t i = 0, end = systemMatrices.size(); i < end; ++i)
            {
                auto M = systemMatrices[i];
                auto rhs = rhsVector[i];

                this->concatenateMatrix(*M,this->A.back(),size,size);
                this->b.back().segment(size,rhs->size());

                size += rhs->size();
            }
        }
    }
}

//---------------------------------------------------------------------------
void Assembler::concatenateMatrix(const core::SparseMatrixd &Q, core::SparseMatrixd &R, std::size_t i, std::size_t j)
{
    for(size_t k = i; k < i+Q.outerSize(); ++k)
    {
        for(core::SparseMatrixd::InnerIterator it(Q,k); it; ++it)
        {
            R.insert(k,it.col()+j) = it.value();
        }
    }
}
