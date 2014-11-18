#include "smCollision/smSpatialHash.h"
#include "smRendering/smGLRenderer.h"
#include "smRendering/smViewer.h"
#include "smCore/smDoubleBuffer.h"
#include "smCore/smSDK.h"
#include "smCollision/smCollisionModel.hpp"
#include "smCore/smGeometry.h"

void smSpatialHash::reset(){
	cells.clearAll();
	cellLines.clearAll();
	cellsForTri2Line.clearAll();
	cellsForModelPoints.clearAll();
	


}
void smSpatialHash::addMesh(smMesh *p_mesh){
	meshes.push_back(p_mesh);
	p_mesh->allocateAABBTris();


}
void smSpatialHash::addMesh(smLineMesh *p_mesh){
	lineMeshes.push_back(p_mesh);
	//p_mesh->allocateAABBTris();


}

void smSpatialHash::removeMesh(smMesh *p_mesh){
	for(smInt i=0;i<meshes.size();i++)
		if(meshes[i]->uniqueId==p_mesh->uniqueId){
			meshes.remove(i);
		}
}

smSpatialHash::~smSpatialHash(){
	delete []   collidedPrims;
	delete []   collidedLineTris;
	delete []   collidedModelPoints;


}
smSpatialHash::smSpatialHash(smErrorLog *p_errorLog,
							 smInt p_hashTableSize,
							 smFloat p_cellSizeX,
							 smFloat p_cellSizeY,
							 smFloat p_cellSizeZ,
							 smInt p_outOutputPrimSize
							 ):smObjectSimulator(p_errorLog),
							 cells(p_hashTableSize),
							 cellLines(p_hashTableSize),
							 cellsForTri2Line(p_hashTableSize),
							 cellsForModel(p_hashTableSize),
							 cellsForModelPoints(p_hashTableSize){

								 cellSizeX=p_cellSizeX;
								 cellSizeY=p_cellSizeY;
								 cellSizeZ=p_cellSizeZ;
								 collidedPrims=new smCollidedTriangles[p_outOutputPrimSize];
								 collidedLineTris=new smCollidedLineTris[p_outOutputPrimSize];
								 collidedModelPoints=new smCollidedModelPoints[p_outOutputPrimSize];

								 //initialize the  number of collisions
								 nbrTriCollisions=0;
								 nbrLineTriCollisions=0;
								 nbrModelPointCollisions=0;
								 
								 maxPrims=p_outOutputPrimSize;
								 
								 pipe=new smPipe("col_hash_tri2line",sizeof(smCollidedLineTris),p_outOutputPrimSize);
								 //smSDK::registerPipe()

								 pipeTriangles=new smPipe("col_hash_tri2tri",sizeof(smCollidedTriangles),p_outOutputPrimSize);

								 pipeModelPoints=new smPipe("col_hash_model2points",sizeof(smCollidedModelPoints),p_outOutputPrimSize);

								 enableDuplicateFilter=false;
							
								 






}


void smSpatialHash::initCustom(){
	smClassType type;
	smSceneObject *object;


	//do nothing for now
	for(smInt i=0;i<meshes.size();i++){
		//meshes[i]->memoryBlocks.allocate<smAABB>(QString("triAABB"),smInt(meshes[i]->nbrTriangles));
		meshes[i]->allocateAABBTris();

	}

}

//void smSpatialHash::computeHash(smLineMesh *p_lineMesh,
void smSpatialHash::computeHash(smMesh *p_mesh,int *p_tris,int p_nbrTris){
	smCellTriangle triangle;
	smFloat xStartIndex,yStartIndex,zStartIndex;											 
	smFloat xEndIndex,yEndIndex,zEndIndex;


	for(int i=0;i<p_nbrTris;i++)
	{
		triangle.primID=p_tris[i];

		xStartIndex=p_mesh->triAABBs[p_tris[i]].aabbMin.x/cellSizeX;             
		yStartIndex=p_mesh->triAABBs[p_tris[i]].aabbMin.y/cellSizeY;             
		zStartIndex=p_mesh->triAABBs[p_tris[i]].aabbMin.z/cellSizeZ;             

		xEndIndex=p_mesh->triAABBs[p_tris[i]].aabbMax.x/cellSizeX;             
		yEndIndex=p_mesh->triAABBs[p_tris[i]].aabbMax.y/cellSizeY;             
		zEndIndex=p_mesh->triAABBs[p_tris[i]].aabbMax.z/cellSizeZ;             



		for(smInt ix=xStartIndex;ix<=xEndIndex;ix++)
			for(smInt iy=yStartIndex;iy<=yEndIndex;iy++)
				for(smInt iz=zStartIndex;iz<=zEndIndex;iz++)
				{
					cells.insert(triangle, HASH(cells.tableSize,ix,iy,iz));        

				}
	}

}
//void smSpatialHash::draw(){
//	smHashIterator<smCellTriangle > iterator;
//	smCellTriangle tri;
//	smGLRenderer::beginTriangles();
//
//	while(cells.next(iterator))
//	{
//
//		while(cells.nextBucketItem(iterator,tri)){
//			glVertex3fv((GLfloat*)&tri.vert[0]);
//			glVertex3fv((GLfloat*)&tri.vert[1]);
//			glVertex3fv((GLfloat*)&tri.vert[2]);
//		}
//	}
//
//
//	glEnd();
//
//
//}

inline void smSpatialHash::addTriangle(smMesh *p_mesh,smInt p_triangleId,smHash<smCellTriangle> &p_cells){
	smFloat xStartIndex,yStartIndex,zStartIndex;
	smFloat  xEndIndex,yEndIndex,zEndIndex;
	smCellTriangle	triangle;			
	triangle.meshID=p_mesh->uniqueId;
	triangle.primID=p_triangleId;
	triangle.vert[0]=p_mesh->vertices[p_mesh->triangles[p_triangleId].vert[0]];
	triangle.vert[1]=p_mesh->vertices[p_mesh->triangles[p_triangleId].vert[1]];
	triangle.vert[2]=p_mesh->vertices[p_mesh->triangles[p_triangleId].vert[2]];

	xStartIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.x/cellSizeX);             
	yStartIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.y/cellSizeY);             
	zStartIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.z/cellSizeZ);             

	xEndIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMax.x/cellSizeX);             
	yEndIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMax.y/cellSizeY);             
	zEndIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMax.z/cellSizeZ);             

	for(smInt ix=xStartIndex;ix<=xEndIndex;ix++)
		for(smInt iy=yStartIndex;iy<=yEndIndex;iy++)
			for(smInt iz=zStartIndex;iz<=zEndIndex;iz++)
				p_cells.checkAndInsert(triangle, HASH(cells.tableSize,ix,iy,iz));      

}



inline smBool smSpatialHash::findCandidateTris(smMesh *p_mesh,smMesh *p_mesh2){
	smAABB aabboverlap;      

	if(smCollisionUtils::checkOverlapAABBAABB(p_mesh->aabb,p_mesh2->aabb,aabboverlap)==false)
		return false;

	for(int i=0;i<p_mesh->nbrTriangles;i++)
	{   
		/*if( smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh->vertices[p_mesh->triangles[i].vert[0]])||
			smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh->vertices[p_mesh->triangles[i].vert[1]])||
			smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh->vertices[p_mesh->triangles[i].vert[2]])){*/
				addTriangle(p_mesh,i,cells);	  
		//}
	}

	for(int i=0;i<p_mesh2->nbrTriangles;i++)
	{   
		/*if(  smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh2->vertices[p_mesh2->triangles[i].vert[0]])||
			smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh2->vertices[p_mesh2->triangles[i].vert[1]])||
			smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh2->vertices[p_mesh2->triangles[i].vert[2]])){*/
				addTriangle(p_mesh2,i,cells);
				/*triangle.primID=i;
				triangle.meshID=p_mesh2->meshId.ID;
				triangle.vert[0]=p_mesh2->vertices[p_mesh2->triangles[i].vert[0]];
				triangle.vert[1]=p_mesh2->vertices[p_mesh2->triangles[i].vert[1]];
				triangle.vert[2]=p_mesh2->vertices[p_mesh2->triangles[i].vert[2]];

				xStartIndex=(smInt)(p_mesh2->triAABBs[i].aabbMin.x/cellSizeX);             
				yStartIndex=(smInt)(p_mesh2->triAABBs[i].aabbMin.y/cellSizeY);             
				zStartIndex=(smInt)(p_mesh2->triAABBs[i].aabbMin.z/cellSizeZ);             

				xEndIndex=(smInt)(p_mesh2->triAABBs[i].aabbMax.x/cellSizeX);             
				yEndIndex=(smInt)(p_mesh2->triAABBs[i].aabbMax.y/cellSizeY);             
				zEndIndex=(smInt)(p_mesh2->triAABBs[i].aabbMax.z/cellSizeZ);             

				for(int ix=xStartIndex;ix<=xEndIndex;ix++)
				for(int iy=yStartIndex;iy<=yEndIndex;iy++)
				for(int iz=zStartIndex;iz<=zEndIndex;iz++)
				cells.insert(triangle, HASH(cells.tableSize,ix,iy,iz));   */     
		//}
	}
	return true;
}



inline void smSpatialHash::addLine(smLineMesh *p_mesh,smInt p_edgeId,smHash<smCellLine> &p_cells){
	smFloat xStartIndex,yStartIndex,zStartIndex;
	smFloat  xEndIndex,yEndIndex,zEndIndex;
	smCellLine	line;			
	line.meshID=p_mesh->uniqueId;
	line.primID=p_edgeId;
	line.vert[0]=p_mesh->vertices[p_mesh->edges[p_edgeId].vert[0]];
	line.vert[1]=p_mesh->vertices[p_mesh->edges[p_edgeId].vert[1]];
	

	xStartIndex=(smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMin.x/cellSizeX);             
	yStartIndex=(smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMin.y/cellSizeY);             
	zStartIndex=(smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMin.z/cellSizeZ);             

	xEndIndex=(smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMax.x/cellSizeX);             
	yEndIndex=(smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMax.y/cellSizeY);             
	zEndIndex=(smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMax.z/cellSizeZ);             

	for(smInt ix=xStartIndex;ix<=xEndIndex;ix++)
		for(smInt iy=yStartIndex;iy<=yEndIndex;iy++)
			for(smInt iz=zStartIndex;iz<=zEndIndex;iz++)
				p_cells.checkAndInsert(line, HASH(cells.tableSize,ix,iy,iz));      

}

inline smBool smSpatialHash::findCandidateTrisLines(smMesh *p_mesh,smLineMesh *p_mesh2){
	smAABB aabboverlap;   
	if(smCollisionUtils::checkOverlapAABBAABB(p_mesh->aabb,p_mesh2->aabb,aabboverlap)==false)
		return false;
	for(int i=0;i<p_mesh->nbrTriangles;i++)
	{   

		if( smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh->triAABBs[i]))
		{

				addTriangle(p_mesh,i,cellsForTri2Line);
				
				
		}
		


	}
	for(int i=0;i<p_mesh2->nbrEdges;i++)
	{   

		if( smCollisionUtils::checkOverlapAABBAABB(aabboverlap,p_mesh2->edgeAABBs[i]))
		{
					
		
			addLine(p_mesh2,i,cellLines);
		}
		


	}


	
	return true;
}


void smSpatialHash::computeCollisionTri2Tri(){
	int t=0;
	smHashIterator<smCellTriangle > iterator;
	smHashIterator<smCellTriangle > iterator1;
	
	smCellTriangle tri;
	smCellTriangle tri1;
	smCollidedTriangles *tristris;
	smVec3f proj1,proj2,inter1,inter2;
	smShort point1,point2;
	smInt coPlanar;

	
	tristris=(smCollidedTriangles*)pipeTriangles->beginWrite();
	while(cells.next(iterator))
	{
		while(cells.nextBucketItem(iterator,tri)){
			iterator1.clone(iterator);
			while(cells.nextBucketItem(iterator1,tri1)){
				if(tri.meshID==tri1.meshID||!(smSDK::getMesh(tri.meshID)->collisionGroup.isCollisionPermitted(smSDK::getMesh(tri1.meshID)->collisionGroup)))
					continue;

				if(t<maxPrims &&smCollisionUtils::tri2tri(tri.vert[0],tri.vert[1],tri.vert[2],tri1.vert[0],tri1.vert[1],tri1.vert[2]
				,coPlanar,inter1,inter2,point1,point2,proj1,proj2))

				{    //cout<<"test"<<endl;
					t++;
					collidedPrims[t].tri1=tri;
					collidedPrims[t].tri2=tri1;
					tristris[t].tri1=tri;
					tristris[t].tri2=tri1;
					tristris[t].proj1=proj1;
					tristris[t].proj2=proj2;
					tristris[t].point1=point1;
					tristris[t].point2=point2;
					
					/*glVertex3fv((GLfloat*)&tri.vert[0]);
					glVertex3fv((GLfloat*)&tri.vert[1]);
					glVertex3fv((GLfloat*)&tri.vert[2]);
					glVertex3fv((GLfloat*)&tri1.vert[0]);
					glVertex3fv((GLfloat*)&tri1.vert[1]);
					glVertex3fv((GLfloat*)&tri1.vert[2]);*/
				}

			}
		}
	}
	pipeTriangles->endWrite(t);
	pipeTriangles->acknowledgeValueListeners();
	nbrTriCollisions=t;

	//cout<<"Collisions:"<<t<<endl;

}

inline smInt compareLineTris(const void* p_element1,const void* p_element2){
	smCollidedLineTris* p_1=(smCollidedLineTris*)p_element1;
	smCollidedLineTris* p_2=(smCollidedLineTris*)p_element2;

	return (p_1->line.primID-p_2->line.primID);


}

inline void  smSpatialHash::filterLine2TrisResults(){
   smCollidedLineTris *lineTris;
   smUInt currentLinePrimID;
   smUInt previousLinePrimID;
   smInt indexofPreviousPrim;
   smInt t=0;
   smBool existed=false;

   lineTris=(smCollidedLineTris*)pipe->beginWrite();
   if(nbrLineTriCollisions<=0)
   {
	    pipe->endWrite(0);
		pipe->acknowledgeValueListeners();
		return;
   
   } 
   if(enableDuplicateFilter)
   {
	   qsort(collidedLineTris,nbrLineTriCollisions,sizeof(smCollidedLineTris),compareLineTris);
	   indexofPreviousPrim=0;
	   previousLinePrimID=collidedLineTris[0].line.primID;

	   for(smInt i=0;i<nbrLineTriCollisions;i++){
		   currentLinePrimID=collidedLineTris[i].line.primID;
			if(previousLinePrimID!=currentLinePrimID){
				indexofPreviousPrim=i;
				previousLinePrimID=currentLinePrimID;
				lineTris[t]=collidedLineTris[i];
				t++;
			}		  
			else{
				existed=false;
				for(smInt j=indexofPreviousPrim;j<i;j++){
					if(collidedLineTris[j].tri.primID==collidedLineTris[i].tri.primID)
					  existed=true;
				}
				if(!existed){
					lineTris[t]=collidedLineTris[i];
					t++;
				}
			}
	  }


   
   }
   else
   {
	   memcpy(lineTris,collidedLineTris,sizeof(smCollidedLineTris)*nbrLineTriCollisions);
	   t=nbrLineTriCollisions;
   
   }
  
  pipe->endWrite(t);
  pipe->acknowledgeValueListeners();

}


///line to triangle collision
void  smSpatialHash::computeCollisionLine2Tri(){
	smInt t=0;
	smHashIterator<smCellLine > iteratorLine;
	smHashIterator<smCellTriangle > iteratorTri;
	smCollidedLineTris *lineTris;
	smCellLine line;
	smCellTriangle tri;
	smVec3<smFloat> intersection;
	
	
	
	//lineTris=(smCollidedLineTris*)pipe->beginWrite();
	while(cellLines.next(iteratorLine)&&cellsForTri2Line.next(iteratorTri)){
		while(cellLines.nextBucketItem(iteratorLine,line)){
		

			iteratorTri.resetBucketIteration();
			while(cellsForTri2Line.nextBucketItem(iteratorTri,tri)){
				
				if(tri.meshID==line.meshID||!(smSDK::getMesh(tri.meshID)->collisionGroup.isCollisionPermitted(smSDK::getMesh(line.meshID)->collisionGroup)))
					continue;
				if(t<maxPrims&&
					smCollisionUtils::checkLineTri(line.vert[0],line.vert[1],
					tri.vert[0],
					tri.vert[1],
					tri.vert[2],intersection))
				{  
					
					collidedLineTris[nbrLineTriCollisions].line=line;
					collidedLineTris[nbrLineTriCollisions].tri=tri;
					collidedLineTris[nbrLineTriCollisions].intersection=intersection;
					nbrLineTriCollisions++;
					//lineTris[t].intersection=intersection;
					//lineTris[t].tri=tri;
					//lineTris[t].line=line;

					

					//t++;
				}

			}
		}
	}
	//pipe->endWrite(t);
    //pipe->acknowledgeValueListeners();
	filterLine2TrisResults();
	//nbrLineTriCollisions=t;


}

void smSpatialHash::initDraw(smDrawParam p_param){

	smViewer *viewer;
	viewer=p_param.rendererObject;
	viewer->addText(QString("smhash"));



}
void smSpatialHash::draw(smDrawParam p_param){


	smViewer *viewer;
	QString fps("Collision FPS: %1 TimePerFrame: %2");
	fps=fps.arg(smDouble(this->FPS)).arg(smDouble(this->timerPerFrame));


	viewer=p_param.rendererObject;
	//viewer->updateText("smhash",fps);

	glDisable(GL_LIGHTING);
	//glDisable(GL_DEPTH_TEST);

	glColor3fv(smColor::colorWhite.toGLColor());
	//for(smInt i=0;i<meshes.size();i++){

	//	//smGLRenderer::draw(meshes[i]->aabb);

	//	for (int j=0;j<meshes[i]->nbrTriangles;j++)
	//		smGLRenderer::draw(meshes[i]->triAABBs[j]);
	//}

	//glutSolidCube(cellSizeX);
	glBegin(GL_TRIANGLES);




	for(smInt i=0;i<nbrTriCollisions;i++){

		glVertex3fv((GLfloat*)&collidedPrims[i].tri1.vert[0]);
		glVertex3fv((GLfloat*)&collidedPrims[i].tri1.vert[1]);
		glVertex3fv((GLfloat*)&collidedPrims[i].tri1.vert[2]);

		glVertex3fv((GLfloat*)&collidedPrims[i].tri2.vert[0]);
		glVertex3fv((GLfloat*)&collidedPrims[i].tri2.vert[1]);
		glVertex3fv((GLfloat*)&collidedPrims[i].tri2.vert[2]);
	}




	glEnd();
	
	glLineWidth(10.0);
	glBegin(GL_LINES);
	for(smInt i=0;i<nbrLineTriCollisions;i++){
		glVertex3fv((GLfloat*)&collidedLineTris[i].line.vert[0]);
		glVertex3fv((GLfloat*)&collidedLineTris[i].line.vert[1]);
		
	
	}
	glEnd();
	glLineWidth(1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);


}


inline void smSpatialHash::addOctreeCell( smSurfaceTree<smOctreeCell> *p_colModel,smHash<smCellModel> p_cells){
	smFloat xStartIndex,yStartIndex,zStartIndex;
	smFloat  xEndIndex,yEndIndex,zEndIndex;
	smCellModel	cellModel;
	smAABB temp;

	smSurfaceTreeIterator<smOctreeCell> iter=p_colModel->get_LevelIterator(); 
	
	cellModel.meshID=p_colModel->getAttachedMeshID();

	for(smInt i=iter.start();i<iter.end();i++){
		if(iter[i].filled){
			
			temp.aabbMin=  iter[i].cube.leftMinCorner();
			temp.aabbMax=  iter[i].cube.rightMaxCorner();
			xStartIndex=(smInt)(temp.aabbMin.x/cellSizeX);             
			yStartIndex=(smInt)(temp.aabbMin.y/cellSizeY);             
			zStartIndex=(smInt)(temp.aabbMin.z/cellSizeZ);   
			xEndIndex=(smInt)(temp.aabbMax.x/cellSizeX);             
			yEndIndex=(smInt)(temp.aabbMax.y/cellSizeY);             
			zEndIndex=(smInt)(temp.aabbMax.z/cellSizeZ);   
			cellModel.primID=i;
			cellModel.center=iter[i].cube.center;
			cellModel.radius=iter[i].cube.getCircumscribedSphere().radius;

			for(smInt ix=xStartIndex;ix<=xEndIndex;ix++)
			 for(smInt iy=yStartIndex;iy<=yEndIndex;iy++)
			  for(smInt iz=zStartIndex;iz<=zEndIndex;iz++)
				p_cells.checkAndInsert(cellModel, HASH(cells.tableSize,ix,iy,iz)); 

		}
	
	}

	          

	    

	
		
	

	//xStartIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.x/cellSizeX);             
	//yStartIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.y/cellSizeY);             
	//zStartIndex=(smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.z/cellSizeZ);             

	

	
	//p_cells.checkAndInsert(triangle, HASH(cells.tableSize,ix,iy,iz));      

}
inline void smSpatialHash::addPoint(smMesh *p_mesh,smInt p_vertId,smHash<smCellPoint> p_cells){
	smFloat xStartIndex,yStartIndex,zStartIndex;
	smFloat  xEndIndex,yEndIndex,zEndIndex;
	smCellPoint	cellPoint;			
	cellPoint.meshID=p_mesh->uniqueId;
	cellPoint.primID=p_vertId;
	cellPoint.vert=p_mesh->vertices[p_vertId];

	xStartIndex=(smInt)(p_mesh->vertices[p_vertId].x/cellSizeX);             
	yStartIndex=(smInt)(p_mesh->vertices[p_vertId].y/cellSizeY);             
	zStartIndex=(smInt)(p_mesh->vertices[p_vertId].z/cellSizeZ);             

	p_cells.checkAndInsert(cellPoint, HASH(cells.tableSize,xStartIndex,yStartIndex,zStartIndex));      

}


void  smSpatialHash::findCandidatePoints(smMesh *p_mesh, smSurfaceTree<smOctreeCell> *p_colModel){
	smAABB tempAABB;
	tempAABB.aabbMin=p_colModel->root.cube.leftMinCorner();
	tempAABB.aabbMax=p_colModel->root.cube.rightMaxCorner();


	for(smInt i=0;i<p_mesh->nbrVertices;i++){
		if(smCollisionUtils::checkAABBPoint(tempAABB,p_mesh->vertices[i]))
			addPoint(p_mesh,i,cellsForModelPoints);
	
	}
	


}

void smSpatialHash::computeCollisionModel2Points(){
	int t=0;
	
	smFloat distanceFromCenter;
	smHashIterator<smCellModel > iteratorModel;
	smHashIterator<smCellPoint > iteratorPoint;
	smCellModel model;
	smCellPoint point;
	smCollidedModelPoints *collidedModelPointsPipe;

	//cout<<"MODEL"<<endl;
	//cellsForModel.printContent();
	//cout<<"Points"<<endl;
	//cellsForModelPoints.printContent();


	collidedModelPointsPipe=(smCollidedModelPoints*)pipeModelPoints->beginWrite();
	while(cellsForModel.next(iteratorModel)&&cellsForModelPoints.next(iteratorPoint)){
		while(cellsForModel.nextBucketItem(iteratorModel,model)){
		

			iteratorPoint.resetBucketIteration();
			while(cellsForModelPoints.nextBucketItem(iteratorPoint,point)){
				
				distanceFromCenter=model.center.distance(point.vert);
				if(t<maxPrims&&distanceFromCenter<model.radius)
				{  
					
					collidedModelPoints[t].penetration=model.radius-distanceFromCenter;
					collidedModelPoints[t].model=model;
					collidedModelPoints[t].point=point;
					
					collidedModelPointsPipe[t].penetration=model.radius-distanceFromCenter;
					collidedModelPointsPipe[t].model=model;
					collidedModelPointsPipe[t].point=point;
					t++;
				}

			}
		}
	}
	
		
	pipeModelPoints->endWrite(t);
	pipeModelPoints->acknowledgeValueListeners();
	nbrModelPointCollisions=t;
	

}

void smSpatialHash::addCollisionModel(smSurfaceTree<smOctreeCell> *p_CollMode ){
	colModel.push_back(p_CollMode);
}
























