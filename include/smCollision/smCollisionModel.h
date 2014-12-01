/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMCOLLISIONMODEL_H
#define SMCOLLISIONMODEL_H

#include <set>
#include "smCore/smConfig.h"
#include "smMesh/smMesh.h"
#include "smCore/smEventHandler.h"
#include "smMesh/smSurfaceMesh.h"
#include "smUtilities/smMatrix44.h"
#include "smCore/smGeometry.h"

#define SOFMIS_TREE_DIVISION_OCTREE 8

//forward declaration..Oteherwise it doesn't compile
class smEvent;
class smSurfaceMesh;

template <typename T>
struct smSurfaceTreeCell{

public:
	smBool filled;
	smShort level;
	set<smInt> verticesIndices;
	vector<smFloat> weights;

	smSurfaceTreeCell(){

		filled=false;
		level=0;
	}

	inline void subDivide(smInt p_divisionPerAxis,T*);
	inline smBool isCollidedWithTri(smVec3f p_v0,smVec3f p_v1,smVec3f p_v2);
	inline smBool isCollidedWithPoint(smVec3f p_point);
	inline void expand(smFloat p_expansion);
	inline void setCenter(smVec3f p_center);
	inline void setLength(smFloat);
	inline void copyShape(T &);
	inline smVec3f getCenter()  const;
	inline smFloat getLength();
};

struct smOctreeCell:public smSurfaceTreeCell<smOctreeCell>{

	smCube cube;
	
	inline smVec3f getCenter() const {
		return  cube.center;
	}

	inline void setCenter(smVec3f p_center){
		cube.center=p_center;
	}

	inline smFloat getLength(){
		return cube.sideLength;
	}

	inline void copyShape(smOctreeCell p_cell){
		cube=p_cell.cube;
	}

	inline void expand(smFloat p_expandScale){
		cube.expand(p_expandScale);
	}

	inline void setLength(smFloat p_length){
		cube.sideLength=p_length;
	}

	inline smBool isCollidedWithTri(smVec3f p_v0,smVec3f p_v1,smVec3f p_v2){
		smAABB tempAABB;
		tempAABB.aabbMin=cube.leftMinCorner();
		tempAABB.aabbMax=cube.rightMaxCorner();
		return smCollisionUtils::checkAABBTriangle(tempAABB,p_v0,p_v1,p_v2);
	}

	inline smBool isCollidedWithPoint(){
	}

	inline void subDivide(smInt p_divisionPerAxis,smOctreeCell *p_cells){

		smInt totalCubes=p_divisionPerAxis*p_divisionPerAxis*p_divisionPerAxis;
		smCube *tempCubes=new smCube[totalCubes];
		cube.subDivide(p_divisionPerAxis,tempCubes);
		for(smInt i=0;i<totalCubes;i++){
			p_cells[i].cube=tempCubes[i];
		}
	}

};



struct smOctree:public smSurfaceTreeCell<smOctree>{

	smOctree(){
		triagleIndices.clear();
		filled=false;
	}

	smCube cube;
	smVec3f originalCubeCenter;
	smSphere sphere;

	vector<smInt> triagleIndices;
	set<smInt> verticesIndices;
	vector<smFloat> weights;

	inline void subDivide(smInt p_division, smOctreeCell *p_cells);
	inline smBool isCollided(smVec3f p_v0,smVec3f p_v1,smVec3f p_v2);
	inline void expand(smFloat p_expansion);
};

struct smLevelIndex{
	smInt startIndex;
	smInt endIndex;
};

enum SOFMIS_TREETYPE{
	SOFMIS_TREETYPE_OCTREE,
	SOFMIS_TREETYPE_CUSTOM
};


template<typename smSurfaceTreeCell> class smSurfaceTree;

template <typename T> class smCollisionModelIterator{

public:
	smInt startIndex;
	smInt currentIndex;
	smInt currentLevel;
	smInt endIndex;

	inline smInt start();
	inline smInt end();
	inline void operator++();
	inline void operator--();
	inline T operator[](smInt p_index);
};


template<typename T> class smCollisionModel:public smCoreClass{

public:
	inline void initStructure();
	inline void reCreateStructure();
	inline void updateStructure();
	void translateRot(); 
	inline void setTranslateRot(smMatrix44d &p_matrix);
	smCollisionModelIterator<T>  get_LevelIterator(smInt p_level) ;
	smCollisionModelIterator<T>  get_LevelIterator() ;
};

template<typename smSurfaceTreeCell>struct smSurfaceTreeIterator:
                             public smCollisionModelIterator<smSurfaceTreeCell>{

	using smCollisionModelIterator<smSurfaceTreeCell>::startIndex;
	using smCollisionModelIterator<smSurfaceTreeCell>::endIndex;
	using smCollisionModelIterator<smSurfaceTreeCell>::currentIndex;

public:
	smSurfaceTree<smSurfaceTreeCell> *tree;

	smSurfaceTreeIterator( smSurfaceTree<smSurfaceTreeCell> *  p_tree){
		tree=p_tree;
	}

	inline void operator++(){
		currentIndex++;
	}

	inline void operator--(){
		currentIndex--;
	}

	inline smSurfaceTreeCell& operator[](smInt p_index)const{
		return tree->treeAllLevels[p_index];
	}

	inline void setLevel(smInt p_level){
		startIndex = tree->levelStartIndex[p_level].startIndex;
		endIndex = tree->levelStartIndex[p_level].endIndex;
	}

	inline void resetIteration(){
		currentIndex = startIndex;
	}

	inline smInt start(){
		return startIndex;
	}

	inline smInt end(){
		return endIndex;
	}

	friend smSurfaceTree<smSurfaceTreeCell>;
};

template<typename smSurfaceTreeCell> class smSurfaceTree:
                            public smCollisionModel<smSurfaceTreeCell>,public smEventHandler{

	protected:
		smSurfaceMesh *mesh;
		smInt nbrDivision;
		smInt minTreeRenderLevel;
		smBool renderSurface;
		smBool enableShiftPos;
		smBool enableTrianglePos;
		smBool renderOnlySurface;
		smInt totalCells;
		smLevelIndex *levelStartIndex;
		smInt currentLevel;
		smBool createTree(smSurfaceTreeCell &p_Node, vector<smInt> &p_triangles,
                          smInt p_level, smInt p_siblingIndex);

	public:
		smMatrix44 <smDouble> transRot;
		smInt maxLevel;
		smSurfaceTreeCell root;
		smFloat shiftScale;

		smSurfaceTreeCell *initialTreeAllLevels;
		smSurfaceTreeCell *treeAllLevels;

		void initDraw(smDrawParam p_param);
		~smSurfaceTree();
		smSurfaceTree(smSurfaceMesh *p_mesh,smInt p_maxLevels,SOFMIS_TREETYPE p_treeType);
		void initStructure();
		smSurfaceTreeIterator<smSurfaceTreeCell>  get_LevelIterator(smInt p_level) ;
		smSurfaceTreeIterator<smSurfaceTreeCell>  get_LevelIterator() ;

		inline smUnifiedID getAttachedMeshID(){
			return mesh->uniqueId;
		}

		virtual void draw(smDrawParam p_params);
		void handleEvent(smEvent *p_event);
		void updateStructure();
		void translateRot();
};

#endif
