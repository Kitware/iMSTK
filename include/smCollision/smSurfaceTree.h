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
#include "smCollision/smSurfaceTreeIterator.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smEventHandler.h"

/// \brief !!
template<typename CellType> 
class smSurfaceTree: public smCollisionModel<CellType>, 
					 public smEventHandler
{
protected:
  typedef smCollisionModel<CellType> BaseType;
  typedef typename BaseType::MatrixType MatrixType;
  
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
    smSurfaceTree(smSurfaceMesh *mesh, int maxLevels);

    /// \brief destructor
    ~smSurfaceTree();
	
    MatrixType transRot; ///< matrix for translation and rotation
    int maxLevel; ///< max level of the tree
    CellType root; ///< !!
    float shiftScale; ///< !!

    std::vector<CellType> initialTreeAllLevels; ///< !!
    std::vector<CellType> treeAllLevels; ///< !!

    /// \brief initialize the draw function related structures
    void initDraw(smDrawParam param);

protected:
  
    /// \brief creates the tree based on input triangles
    bool createTree(CellType &Node, const std::vector<int> &triangles,
                      int level, int siblingIndex);

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
    virtual void draw(smDrawParam params);

    /// \brief !!
    void handleEvent(smEvent *event);

    /// \brief !! smSurfaceTree structure
    void updateStructure();

    /// \brief !!
    void translateRot();
};

#include "smCollision/smSurfaceTree.hpp"

#endif
