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

// SimMedTK includes
#include "smCollision/smPlaneToMeshCollision.h"
#include "smCollision/smCollisionMoller.h"
#include "smCollision/smCollisionPair.h"
#include "smCollision/smMeshCollisionModel.h"

// STL includes
#include <vector>

void smPlaneToMeshCollision::doComputeCollision(std::shared_ptr<smCollisionPair> pairs)
{
    auto mesh = std::static_pointer_cast<smMeshCollisionModel>(pairs->getFirst());
    auto plane = std::static_pointer_cast<smpPlaneCollisionModel>(pairs->getFirst());

    if(!mesh || !plane)
        return;
    
    int nodeID;
    double depth;
    smVec3d normal;
    smVec3d contactPoint;

/*
    for (int i=0; i<collisionModel->getMesh()-> ; i++)
    {

    }*/
   
}

std::vector<std::pair<std::shared_ptr<CellType>,std::shared_ptr<CellType>>>
    getIntersectingNodes(std::shared_ptr<smSurfaceTree<CellType>> otherTree)
    {
        std::vector<std::pair<std::shared_ptr<CellType>,std::shared_ptr<CellType>>> intersectingNodes;
        getIntersectingNodes(root, otherTree->getRoot(),intersectingNodes);

        return intersectingNodes;
    }

    void getIntersectingNodes(const std::shared_ptr<CellType> left,
                              const std::shared_ptr<smpPlaneCollisionModel> right,
                              std::vector<std::pair<std::shared_ptr<CellType>>> &result )
    {
        if(!smCollisionMoller::checkOverlapAABBAABB(left->getAabb(),right->getAabb()))
        {
            return;
        }

        if(left->getIsLeaf() && right->getIsLeaf())
        {
            result.emplace_back(left,right);
        }
        else if(left->getIsLeaf())
        {
            for(const auto &child : right->getChildNodes())
            {
                if(!child) continue;
                getIntersectingNodes(left,child,result);
            }
        }
        else if(right->getIsLeaf())
        {
            for(const auto &child : left->getChildNodes())
            {
                if(!child) continue;
                getIntersectingNodes(child,right,result);
            }
        }
        else
        {
            for(const auto &rightChild : right->getChildNodes())
            {
                if(!rightChild) continue;
                for(const auto &leftChild : left->getChildNodes())
                {
                    if(!leftChild) continue;
                    getIntersectingNodes(leftChild,rightChild,result);
                }
            }
        }

    }
