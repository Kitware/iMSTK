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

#ifndef SM_SMSURFACETREE_H
#define SM_SMSURFACETREE_H

// STD includes
#include <vector>
#include <array>

// SimMedTK includes
#include "smCollision/smCollisionModel.h"
#include "smCollision/smSurfaceTreeCell.h"
#include "smCollision/smCollisionMoller.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smEventHandler.h"

/// \brief !!
template<typename CellType>
class smSurfaceTree
{
protected:
  typedef smMatrix44f MatrixType;

protected:
    smSurfaceMesh *mesh; 							///< surface mesh
    int minTreeRenderLevel; 						///< !!
    bool renderSurface; 							///< !!
    bool enableShiftPos; 							///< !!
    bool enableTrianglePos; 						///< !!
    bool renderOnlySurface; 						///< true if rendering only the surface
    int totalCells; 								///< number of total cells
    std::vector<std::array<int,2>> levelStartIndex; ///< Stores levels start and end indices
    int currentLevel; ///<

public:

    /// \brief constructor
    smSurfaceTree(smSurfaceMesh *mesh, int maxLevels = 6);

    /// \brief destructor
    ~smSurfaceTree();

    MatrixType transRot; ///< matrix for translation and rotation
    int maxLevel; ///< max level of the tree
    CellType root; ///< !!
    float shiftScale; ///< !!

    std::vector<CellType> initialTreeAllLevels; ///< !!
    std::vector<CellType> treeAllLevels; ///< !!

    /// \brief initialize the draw function related structures
    void initDraw(const smDrawParam &param);

protected:

    /// \brief creates the tree based on input triangles
    bool createTree(smSurfaceTreeCell<CellType> &Node,
                    const std::vector<int> &triangles,
                    int siblingIndex);

public:

    /// \brief initialize the surface tree structure
    virtual void initStructure();

    virtual smCollisionModelIterator<CellType>  getLevelIterator(int level) ;

    virtual smCollisionModelIterator<CellType>  getLevelIterator() ;

    /// \brief !!
    inline smUnifiedID getAttachedMeshID()
    {
        return mesh->uniqueId;
    }

    /// \brief rendering the surface tree
    virtual void draw(const smDrawParam &params);

    /// \brief !!
    void handleEvent(std::shared_ptr<smEvent> p_event);

    /// \brief !! smSurfaceTree structure
    void updateStructure();

    /// \brief !!
    void translateRot();

    CellType &getRoot()
    {
        return root;
    }

    std::vector<std::pair<CellType,CellType>> getIntersectingNodes(std::shared_ptr<smSurfaceTree<CellType>> otherTree)
    {
        std::vector<std::pair<CellType,CellType>> intersectingNodes;
        getIntersectingNodes(root, otherTree->getRoot(),otherTree,intersectingNodes);

        return intersectingNodes;
    }

    void getIntersectingNodes(const CellType &left,
                              const CellType &right,
                              std::shared_ptr<smSurfaceTree<CellType>> otherTree,
                              std::vector<std::pair<CellType,CellType>> &result )
    {
        if(!smCollisionMoller::checkOverlapAABBAABB(left.getAabb(),right.getAabb()))
        {
            return;
        }

        if(left.getIsLeaf() && right.getIsLeaf())
        {
            result.emplace_back(left,right);
        }
        else if(left.getIsLeaf())
        {
            auto rightIterator = otherTree->getLevelIterator(right.getLevel());
            for(auto i = rightIterator.start(); i != rightIterator.end(); ++i)
            {
                getIntersectingNodes(left,rightIterator[i],otherTree,result);
            }
        }
        else if(right.getIsLeaf())
        {
            auto leftIterator = getLevelIterator(left.getLevel());
            for(auto i = leftIterator.start(); i != leftIterator.end(); ++i)
            {
                getIntersectingNodes(leftIterator[i],right,otherTree,result);
            }
        }
        else
        {
            auto rightIterator = otherTree->getLevelIterator(right.getLevel());
            for(auto i = rightIterator.start(); i != rightIterator.end(); ++i)
            {
                auto leftIterator = getLevelIterator(left.getLevel());
                for(auto j = leftIterator.start(); j != leftIterator.end(); ++j)
                {
                    getIntersectingNodes(leftIterator[i],rightIterator[i],otherTree,result);
                }
            }
        }

    }
};

#include "smCollision/smSurfaceTree.hpp"

#endif
