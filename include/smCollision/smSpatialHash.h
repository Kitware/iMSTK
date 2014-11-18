/*
****************************************************
SOFMIS LICENSE

****************************************************

\author:    <http:\\acor.rpi.edu>
SOFMIS TEAM IN ALPHABATIC ORDER
Anderson Maciel, Ph.D.
Ganesh Sankaranarayanan, Ph.D.
Sreekanth A Venkata
Suvranu De, Ph.D.
Tansel Halic
Zhonghua Lu

\author:    Module by Tansel Halic


\version    1.0
\date       05/2009
\bug	    None yet
\brief	    This class is the simulator object. Each simulator should derive this.



*****************************************************
*/

#ifndef SMSPATIALHASH_H
#define SMSPATIALHASH_H

#include <QVector>

#include "smCore/smConfig.h"
#include "smCollision/smCollisionConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smDataStructs.h"
#include "smCore/smPipe.h"

class smPipe;
template<typename T> class smCollisionModel;
template<typename smSurfaceTreeCell> class smSurfaceTree;
struct smOctreeCell;
template<typename smSurfaceTreeCell> struct smSurfaceTreeIterator;

typedef smHash<smCellPoint> smHashCellPoint;



	

enum smCollisionSetting{
	SOFMIS_COLISIONSETTING_MODEL2POINT,
	SOFMIS_COLISIONSETTING_MODEL2TRIANGLE,
	SOFMIS_COLISIONSETTING_MODEL2MODEL
};


#define SOFMIS_COLLISIONOUTPUTBUF_SIZE 1000
#define HASH_P1 73856093
#define HASH_P2 19349663
#define HASH_P3 83492791

inline unsigned int HASH(unsigned int p_SIZE, unsigned int p_x, unsigned int p_y, unsigned int p_z){
	return ((( ((p_x)*HASH_P1) ^ ((p_y)*HASH_P2) ^ ((p_z)*HASH_P3))) % (p_SIZE));

}







///example simulator..the dummy simulator works on static scene objects for now..
class smSpatialHash:public smObjectSimulator{

protected:
	smFloat cellSizeX;
	smFloat cellSizeY;
	smFloat cellSizeZ;

	//These structures below are Triangle2Triangle collisions
	
	///Cells for triangles. It stores candidate triangles
	smHash<smCellTriangle> cells;
	///structure that stores the meshes in the scene
	QVector<smMesh *> meshes;
	///After the collision is cimpleted the result is written in here
	smCollidedTriangles *collidedPrims;
	///Number of collisions that triangles are stored.	
	smInt nbrTriCollisions;

	///Line mesh structure that is added to collision detection engine
	QVector<smLineMesh *> lineMeshes;
	///Lines that stored in the scene.
	smHash<smCellLine> cellLines;
	///candidate triangles in the scene.
	smHash<smCellTriangle> cellsForTri2Line;


	///candidate cells for collision model
	smHash<smCellModel> cellsForModel;

	///candidate for Collision model to point 
	smHash<smCellPoint> cellsForModelPoints;




	///The result is stored here.
	smCollidedLineTris *collidedLineTris;
	///The number of collisions that for line to triangle.
	smInt nbrLineTriCollisions;

	//smHash<smCollisionGroup> collisionGroups;
	
	///the collision results are here
	smCollidedModelPoints *collidedModelPoints;
	
	///the number of collisions for model to point
	smInt nbrModelPointCollisions;

	

	
	///For maximum collision output.
	smInt maxPrims;

	map<smInt,smInt> filteredList; 
	
	
	
	
	

	inline void addTriangle(smMesh *p_mesh,smInt p_triangleId,smHash<smCellTriangle> &p_cells);
	inline void addLine(smLineMesh *p_mesh,smInt p_edgeId,smHash<smCellLine> &p_cells);
	
	inline void addPoint(smMesh *p_mesh,smInt p_vertId,smHash<smCellPoint> p_cells);
	inline void addOctreeCell( smSurfaceTree<smOctreeCell> *p_colModel,smHash<smCellModel> p_cells);
	
	void reset();

	vector<smSurfaceTree<smOctreeCell>*> colModel;
public:
	 smBool enableDuplicateFilter;
	 smPipe *pipe;
	 smPipe *pipeTriangles;
	 smPipe *pipeModelPoints;

	 
	 void  findCandidatePoints(smMesh *p_mesh, smSurfaceTree<smOctreeCell> *p_colModel);
	 void computeCollisionModel2Points();

	~smSpatialHash();

	void addCollisionModel(smSurfaceTree<smOctreeCell> *p_CollMode );
	void addMesh(smMesh *mesh);
	void addMesh(smLineMesh *p_mesh);
	void removeMesh(smMesh *p_mesh);
	smSpatialHash(smErrorLog *p_errorLog,
	              smInt p_hashTableSize,
	              smFloat p_cellSizeX,
	              smFloat p_cellSizeY,
	              smFloat p_cellSizeZ,
	              smInt p_outOutputPrimSize = SOFMIS_COLLISIONOUTPUTBUF_SIZE);

	void draw();		
	smBool findCandidateTris(smMesh *p_mesh,smMesh  *p_mesh2);
	smBool findCandidateTrisLines(smMesh *p_mesh,smLineMesh *p_mesh2);
	void computeCollisionTri2Tri();
	void computeCollisionLine2Tri();
	void filterLine2TrisResults();

	
	void draw(smDrawParam p_param);
	void initDraw(smDrawParam p_param);


	virtual void beginSim(){
		smObjectSimulator::beginSim();
		//start the job
		nbrTriCollisions=0;
		nbrLineTriCollisions=0;
		nbrModelPointCollisions=0;
		for(smInt i=0;i<meshes.size();i++)
			meshes[i]->updateTriangleAABB();
		
		for(smInt i=0;i<lineMeshes.size();i++)
			meshes[i]->upadateAABB();
			
		
		
	}


	virtual void initCustom();
	//{
	//	smClassType type;  
	//	smSceneObject *object;
	//	
	//	smVec3<smFloat> *newVertices;
	//	//do nothing for now
	//	for(smInt i=0;i<objectsSimulated.size();i++){
	//	
	//	}

	//}

	inline void computeHash(smMesh *p_mesh,int *p_tris,int p_nbrTris);
	
	//test
	virtual void run(){
		
		smTimer timer;
		timer.start();
		beginSim();

		
		//cout<<"Time:"<<timer.now(SOFMIS_TIMER_INMILLISECONDS)<<endl;
		for(smInt i=0;i<colModel.size();i++)
			
			for(smInt i=0;i<meshes.size();i++){
				findCandidatePoints(meshes[i],colModel[i]);
			addOctreeCell(colModel[i], cellsForModel);
		}
		
		

		///Triangle-Triangle collision
		for(smInt i=0;i<meshes.size();i++){
			
			for(smInt j=i+1;j<meshes.size();j++)
			{
				if(meshes[i]->collisionGroup.isCollisionPermitted(meshes[j]->collisionGroup)){
					if(findCandidateTris(meshes[i],meshes[j])==false)	
						continue;
				}
			}
		}

	  
		///Triangle-line Collision
		for(smInt i=0;i<meshes.size();i++)
			for(smInt j=0;j<lineMeshes.size();j++){

				if(meshes[i]->collisionGroup.isCollisionPermitted(lineMeshes[j]->collisionGroup)){
					if(findCandidateTrisLines(meshes[i],lineMeshes[j])==false)
						continue;
				}
			
			
		}
		 
		
		computeCollisionTri2Tri();
		computeCollisionLine2Tri();
		computeCollisionModel2Points();
		
		endSim();
	}

	void endSim(){
		//end the job
		smObjectSimulator::endSim();
	   reset();
	}

	///synchronize the buffers in the object..do not call by yourself.
	void syncBuffers(){





	}



};










#endif