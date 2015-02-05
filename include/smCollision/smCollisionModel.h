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

#ifndef SMCOLLISIONMODEL_H
#define SMCOLLISIONMODEL_H

#include <set>
#include "smCore/smConfig.h"
#include "smMesh/smMesh.h"
#include "smCore/smEventHandler.h"
#include "smMesh/smSurfaceMesh.h"
#include "smUtilities/smMatrix44.h"
#include "smCore/smGeometry.h"

#define SIMMEDTK_TREE_DIVISION_OCTREE 8

//forward declaration..Otherwise it doesn't compile
class smEvent;
class smSurfaceMesh;

/// \brief contains the cell of the surface tree structure
template <typename T> struct smSurfaceTreeCell
{

public:
    smBool filled; ///< !!
    smShort level; ///< level in the tree
    set<smInt> verticesIndices; ///< indices of vertices
    vector<smFloat> weights; ///< !!

    /// \brief constructor
    smSurfaceTreeCell()
    {

        filled = false;
        level = 0;
    }

    /// \brief subdivide the cell of surface tree structure
    inline void subDivide(smInt p_divisionPerAxis, T*);

    /// \brief checks if the cell collided with a triangle primitive
    inline smBool isCollidedWithTri(smVec3f p_v0, smVec3f p_v1, smVec3f p_v2);

    /// \brief checks if the cell contains the point primitive
    inline smBool isCollidedWithPoint(smVec3f p_point);

    /// \brief !! expand the cell of the surface tree structure
    inline void expand(smFloat p_expansion);

    /// \brief set the center of the cell of surface tree
    inline void setCenter(smVec3f p_center);

    /// \brief set the length of the cell of surface tree
    inline void setLength(smFloat);

    /// \brief !! copy the cell shape
    inline void copyShape(T &);

    /// \brief get the center of the cell of surface tree
    inline smVec3f getCenter()  const;

    /// \brief set the length of the cell of surface tree
    inline smFloat getLength();
};

/// \brief cell of an octree
struct smOctreeCell: public smSurfaceTreeCell<smOctreeCell>
{

    smCube cube; ///< cube

    /// \brief get the center of the octree cell
    inline smVec3f getCenter() const
    {
        return  cube.center;
    }

    /// \brief set the center of the octree cell
    inline void setCenter(smVec3f p_center)
    {
        cube.center = p_center;
    }

    /// \brief get the side length of the octree cell
    inline smFloat getLength()
    {
        return cube.sideLength;
    }

    /// \brief set the octree cell
    inline void copyShape(smOctreeCell p_cell)
    {
        cube = p_cell.cube;
    }

    /// \brief !! expand the cell of the octree structure
    inline void expand(smFloat p_expandScale)
    {
        cube.expand(p_expandScale);
    }

    /// \brief set the length of the octree cell
    inline void setLength(smFloat p_length)
    {
        cube.sideLength = p_length;
    }

    /// \brief check if a triangle is intersecting the octree cell
    inline smBool isCollidedWithTri(smVec3f p_v0, smVec3f p_v1, smVec3f p_v2)
    {
        smAABB tempAABB;
        tempAABB.aabbMin = cube.leftMinCorner();
        tempAABB.aabbMax = cube.rightMaxCorner();
        return smCollisionUtils::checkAABBTriangle(tempAABB, p_v0, p_v1, p_v2);
    }

    /// \brief check if a point lies inside an octree cell
    inline smBool isCollidedWithPoint()
    {
    }

    /// \brief subdivide the cells of octree cells
    inline void subDivide(smInt p_divisionPerAxis, smOctreeCell *p_cells)
    {

        smInt totalCubes = p_divisionPerAxis * p_divisionPerAxis * p_divisionPerAxis;
        smCube *tempCubes = new smCube[totalCubes];
        cube.subDivide(p_divisionPerAxis, tempCubes);

        for (smInt i = 0; i < totalCubes; i++)
        {
            p_cells[i].cube = tempCubes[i];
        }
    }

};

/// \brief octree
struct smOctree: public smSurfaceTreeCell<smOctree>
{

    /// \brief constructor
    smOctree()
    {
        triagleIndices.clear();
        filled = false;
    }

    smCube cube; ///< cube
    smVec3f originalCubeCenter; ///< original cube center
    smSphere sphere; ///< !!

    vector<smInt> triagleIndices;  ///< triangle indices
    set<smInt> verticesIndices;  ///< vertices indices
    vector<smFloat> weights; /// !!

    /// \brief subdivide octree
    inline void subDivide(smInt p_division, smOctreeCell *p_cells);

    /// \brief !!
    inline smBool isCollided(smVec3f p_v0, smVec3f p_v1, smVec3f p_v2);

    /// \brief !!
    inline void expand(smFloat p_expansion);
};

/// \brief !!
struct smLevelIndex
{
    smInt startIndex;
    smInt endIndex;
};

/// \brief !!
enum SIMMEDTK_TREETYPE
{
    SIMMEDTK_TREETYPE_OCTREE,
    SIMMEDTK_TREETYPE_CUSTOM
};


template<typename smSurfaceTreeCell> class smSurfaceTree;

/// \brief !!
template <typename T> class smCollisionModelIterator
{

public:
    smInt startIndex; ///<
    smInt currentIndex; ///<
    smInt currentLevel; ///<
    smInt endIndex; ///<

    inline smInt start();
    inline smInt end();
    inline void operator++();
    inline void operator--();
    inline T operator[](smInt p_index);
};

/// \brief !!
template<typename T> class smCollisionModel: public smCoreClass
{

public:
    /// \brief !!
    inline void initStructure();

    /// \brief !!
    inline void reCreateStructure();

    /// \brief !!
    inline void updateStructure();

    /// \brief !!
    void translateRot();

    /// \brief !!
    inline void setTranslateRot(smMatrix44d &p_matrix);

    /// \brief !!
    smCollisionModelIterator<T>  get_LevelIterator(smInt p_level) ;

    /// \brief !!
    smCollisionModelIterator<T>  get_LevelIterator() ;
};

/// \brief !!
template<typename smSurfaceTreeCell>struct smSurfaceTreeIterator:
        public smCollisionModelIterator<smSurfaceTreeCell>
{

    using smCollisionModelIterator<smSurfaceTreeCell>::startIndex;
    using smCollisionModelIterator<smSurfaceTreeCell>::endIndex;
    using smCollisionModelIterator<smSurfaceTreeCell>::currentIndex;

public:
    smSurfaceTree<smSurfaceTreeCell> *tree;

    /// \brief
    smSurfaceTreeIterator(smSurfaceTree<smSurfaceTreeCell> *  p_tree)
    {
        tree = p_tree;
    }

    /// \brief up the index
    inline void operator++()
    {
        currentIndex++;
    }

    /// \brief lower the index
    inline void operator--()
    {
        currentIndex--;
    }

    inline smSurfaceTreeCell& operator[](smInt p_index)const
    {
        return tree->treeAllLevels[p_index];
    }

    /// \brief !!
    inline void setLevel(smInt p_level)
    {
        startIndex = tree->levelStartIndex[p_level].startIndex;
        endIndex = tree->levelStartIndex[p_level].endIndex;
    }

    /// \brief !!
    inline void resetIteration()
    {
        currentIndex = startIndex;
    }

    /// \brief !!
    inline smInt start()
    {
        return startIndex;
    }

    /// \brief !!
    inline smInt end()
    {
        return endIndex;
    }

    friend smSurfaceTree<smSurfaceTreeCell>;
};

/// \brief !!
template<typename smSurfaceTreeCell> class smSurfaceTree:
    public smCollisionModel<smSurfaceTreeCell>, public smEventHandler
{

protected:
    smSurfaceMesh *mesh; ///< surface mesh
    smInt nbrDivision; ///< !!
    smInt minTreeRenderLevel; ///< !!
    smBool renderSurface; ///< !!
    smBool enableShiftPos; ///< !!
    smBool enableTrianglePos; ///< !!
    smBool renderOnlySurface; ///< true if rendering only the surface
    smInt totalCells; ///< number of total cells
    smLevelIndex *levelStartIndex; ///<
    smInt currentLevel; ///<

    /// \brief creates the tree based on input triangles
    smBool createTree(smSurfaceTreeCell &p_Node, vector<smInt> &p_triangles,
                      smInt p_level, smInt p_siblingIndex);

public:
    smMatrix44 <smDouble> transRot; ///< matrix for translation and rotation
    smInt maxLevel; ///< max level of the tree
    smSurfaceTreeCell root; ///< !!
    smFloat shiftScale; ///< !!

    smSurfaceTreeCell *initialTreeAllLevels; ///< !!
    smSurfaceTreeCell *treeAllLevels; ///< !!

    /// \brief initialize the draw function related structures
    void initDraw(smDrawParam p_param);

    /// \brief destructor
    ~smSurfaceTree();

    /// \brief constructor
    smSurfaceTree(smSurfaceMesh *p_mesh, smInt p_maxLevels, SIMMEDTK_TREETYPE p_treeType);

    /// \brief initialize the surface tree structure
    void initStructure();

    smSurfaceTreeIterator<smSurfaceTreeCell>  get_LevelIterator(smInt p_level) ;

    smSurfaceTreeIterator<smSurfaceTreeCell>  get_LevelIterator() ;

    /// \brief !!
    inline smUnifiedID getAttachedMeshID()
    {
        return mesh->uniqueId;
    }

    /// \brief rendering the surface tree
    virtual void draw(smDrawParam p_params);

    /// \brief !!
    void handleEvent(smEvent *p_event);

    /// \brief !! smSurfaceTree structure
    void updateStructure();

    /// \brief !!
    void translateRot();
};

#endif
