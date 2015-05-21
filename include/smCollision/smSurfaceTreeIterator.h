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

#ifndef SM_SMSURFACETREEITERATOR_H
#define SM_SMSURFACETREEITERATOR_H

// SimMedTK includes
#include "smCollision/smCollisionModelIterator.h"
#include "smCollision/smSurfaceTree.h"

/// \brief !!
template<typename CellType>
class smSurfaceTreeIterator: public smCollisionModelIterator<CellType>
{

public:
    smSurfaceTree<CellType> *tree;

    /// \brief
    smSurfaceTreeIterator(smSurfaceTree<CellType> *  p_tree)
    {
        tree = p_tree;
    }

    /// \brief
    smSurfaceTreeIterator(const smCollisionModelIterator<CellType> &i)
	  : smCollisionModelIterator<CellType>()
    {
		this->tree = nullptr;
		const smSurfaceTreeIterator<CellType> &otherTree = static_cast<const smSurfaceTreeIterator<CellType>&>(i);
		if(otherTree.tree)
		  this->tree = otherTree.tree;
		this->startIndex = i.startIndex;
		this->currentIndex = i.currentIndex;
		this->currentLevel = i.currentLevel;
		this->endIndex = i.endIndex;
    }

    /// \brief up the index
    inline void operator++()
    {
        this->currentIndex++;
    }

    /// \brief lower the index
    inline void operator--()
    {
        this->currentIndex--;
    }

    inline CellType& operator[](int p_index)const
    {
        return tree->treeAllLevels[p_index];
    }

    /// \brief !!
    inline void setLevel(int p_level)
    {
        this->startIndex = tree->levelStartIndex[p_level].startIndex;
        this->endIndex = tree->levelStartIndex[p_level].endIndex;
    }

    /// \brief !!
    inline void resetIteration()
    {
        this->currentIndex = this->startIndex;
    }

    /// \brief !!
    inline int start()
    {
        return this->startIndex;
    }

    /// \brief !!
    inline int end()
    {
        return this->endIndex;
    }

    friend smSurfaceTree<CellType>;
};

#endif
