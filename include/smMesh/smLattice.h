/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMLATTICE_H
#define SMLATTICE_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smUtilities/smVec3.h"
#include "smRendering/smCustomRenderer.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smSceneObject.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smGeometry.h"

#define SOFMIS_SPATIALGRID_MAXPRIMITIVES 500
#define SOFMIS_SPATIALGRID_MAXCELLS 1000
#define SOFMIS_SMLATTICE_NONE					(0)
#define SOFMIS_SMLATTICE_ALL					(1<<1)
#define SOFMIS_SMLATTICE_MINMAXPOINTS			(1<<2)
#define SOFMIS_SMLATTICE_SEPERATIONLINES		(1<<3)
#define SOFMIS_SMLATTICE_CELLS					(1<<4)
#define SOFMIS_SMLATTICE_CELLCENTERS			(1<<5)
#define SOFMIS_SMLATTICE_CELLPOINTS				(1<<6)
#define SOFMIS_SMLATTICE_CELLPOINTSLINKS		(1<<7)
#define SOFMIS_SMLATTICE_CENTER					(1<<8)
#define SOFMIS_SMLATTICE_CELLVERTICES			(1<<9)
#define SOFMIS_SMLATTICE_CELLACTIVEVERTICES		(1<<10)
#define SOFMIS_SMLATTICE_CELLTRIANGLES			(1<<11)

enum smLatticeReturnType{
	SOFMIS_LATTICE_OK,
	SOFMIS_LATTICE_INVALIDPARAMS,
	SOFMIS_LATTICE_INVALIDBOUNDS
};

struct smCollisionPairs {
	smUnifiedID objectIndex;
	smUnifiedID objectIndex2;
	smInt primIndex;
	smInt primIndex2;
};

struct smCellPrim{
	smInt index;
	smInt objectId;
};

class smCell{

public:
	smInt id;
	smInt cellId[3];
	smVec3<smFloat> cellCenter;
	smVec3<smFloat> cellLeftCorner;
	smVec3<smFloat> cellRightCorner;
	smCellPrim cellPrimitives[SOFMIS_SPATIALGRID_MAXPRIMITIVES];
	smInt lastPrimitiveIndex;
	smInt timeStamp;
	smBool isActive;

	smCell(){
	}
};

class smLattice:public smCoreClass{

public:
	//these should be templated..Current design is based on the triangle
	smAABB *aabb;
	smSurfaceMesh *mesh;
	smCell *cells;
	smInt totalCells;
	smInt xSeperation;
	smInt ySeperation;
	smInt zSeperation;
	smFloat xStep;
	smFloat yStep;
	smFloat zStep;
	smVec3<smFloat> latticeCenter;
	smInt time;
	smUnifiedID linkedObject;

	//template <class smPrimitive>
	void boundingBoxInit(){
		aabb=new smAABB[mesh->nbrTriangles];
	}

	smLattice (){
		this->cells=NULL;
		this->totalCells=0;
		this->xStep=0;
		this->yStep=0;
		this->zStep=0;
		this->xSeperation=0;
		this->ySeperation=0;
		this->zSeperation=0;
	}

	inline smFloat getXStep(){return xStep; }
	inline smFloat getYStep(){return yStep; }
	inline smFloat getZStep(){return zStep; }
	inline smVec3<smFloat> getLatticeCenter(){return latticeCenter;}
	inline smVec3<smFloat> getLeftMinCorner(){return cells[0].cellLeftCorner;}
	inline smVec3<smFloat> getRightMaxCorner(){return cells[totalCells-1].cellRightCorner;}

	~smLattice(){
		delete[] cells;
		delete[] aabb;
	}

	smLatticeReturnType init(smVec3<smFloat> p_leftCorner,smVec3<smFloat> p_rightCorner,
                             smInt p_xSeperation, smInt p_ySeperation, smInt p_zSeperation ){

		smInt x,y,z;
		smInt index;

		xSeperation=p_xSeperation;
		ySeperation=p_ySeperation;
		zSeperation=p_zSeperation;

		boundingBoxInit();
		cells=new smCell[xSeperation*ySeperation*zSeperation];
		zStep= (p_rightCorner.z-p_leftCorner.z)/zSeperation;
		yStep= (p_rightCorner.y-p_leftCorner.y)/ySeperation;
		xStep= (p_rightCorner.x-p_leftCorner.x)/xSeperation;
		smInt counter=0;
		for( y=0;y<ySeperation;y++)
			for( z=0;z<zSeperation;z++)
				for( x=0;x<xSeperation;x++){
					index=x+z*xSeperation+y*xSeperation*zSeperation;
					if(x<0||y<0|z<0||x>=xSeperation||y>=ySeperation||z>=zSeperation){
						printf("Error index is out of bounds in createllatice function");
						return SOFMIS_LATTICE_INVALIDBOUNDS;
					}
					cells[index].id=index;	
					cells[index].cellLeftCorner[0]=p_leftCorner[0]+x*xStep;
					cells[index].cellLeftCorner[1]=p_leftCorner[1]+y*yStep;
					cells[index].cellLeftCorner[2]=p_leftCorner[2]+z*zStep;

					cells[index].cellRightCorner[0]=cells[index].cellLeftCorner[0]+xStep;
					cells[index].cellRightCorner[1]=cells[index].cellLeftCorner[1]+yStep;
					cells[index].cellRightCorner[2]=cells[index].cellLeftCorner[2]+zStep;


					cells[index].cellCenter[0]=(cells[index].cellLeftCorner[0]+cells[index].cellRightCorner[0])/2;
					cells[index].cellCenter[1]=(cells[index].cellLeftCorner[1]+cells[index].cellRightCorner[1])/2;
					cells[index].cellCenter[2]=(cells[index].cellLeftCorner[2]+cells[index].cellRightCorner[2])/2;
					cells[index].isActive=false;
					cells[index].lastPrimitiveIndex=0;

					for(smInt j=0;j<SOFMIS_SPATIALGRID_MAXPRIMITIVES;j++){
						cells[index].cellPrimitives[j].index=0;
					}
					counter++;
				}
				this->totalCells=counter;
				this->xStep=xStep;
				this->yStep=yStep;
				this->zStep=zStep;
				this->xSeperation=xSeperation;
				this->ySeperation=ySeperation;
				this->zSeperation=zSeperation;
				this->latticeCenter[0]=(p_leftCorner[0]+p_rightCorner[0])/2.0;
				this->latticeCenter[1]=(p_leftCorner[1]+p_rightCorner[1])/2.0;
				this->latticeCenter[2]=(p_leftCorner[2]+p_rightCorner[2])/2.0;

				return SOFMIS_LATTICE_OK;
	}

	void indexReset(){

		int traverseIndex=0;

		for(int y=0;y<ySeperation;y++)
			for(int z=0;z<zSeperation;z++)
				for(int x=0;x<xSeperation;x++){
					traverseIndex=x+z*xSeperation+y*xSeperation*zSeperation;
					cells[traverseIndex].lastPrimitiveIndex=0;
				}
	}

	void inline isCellEmpty(smInt p_cellIndex){
	}

	inline virtual void  linkPrimitivetoCell(smInt p_primitiveIndex){

		smInt minX;
		smInt minY;
		smInt minZ;
		smInt maxX;
		smInt maxY;
		smInt maxZ;
		smInt index;
		smVec3<smFloat> leftCorner=getLeftMinCorner();
		smVec3<smFloat> rightCorner=getRightMaxCorner();

		minX=(aabb[p_primitiveIndex].aabbMin.x-leftCorner[0])/xStep;
		minY=(aabb[p_primitiveIndex].aabbMin.y-leftCorner[1])/yStep;
		minZ=(aabb[p_primitiveIndex].aabbMin.z-leftCorner[2])/zStep;

		maxX=(aabb[p_primitiveIndex].aabbMax.x-leftCorner[0])/xStep;
		maxY=(aabb[p_primitiveIndex].aabbMax.y-leftCorner[1])/yStep;
		maxZ=(aabb[p_primitiveIndex].aabbMax.z-leftCorner[2])/zStep;

		for(smInt yIndex=minY;yIndex<=maxY;yIndex++)
			for(smInt xIndex=minX;xIndex<=maxX;xIndex++)
				for(smInt zIndex=minZ;zIndex<=maxZ;zIndex++){
					index=xIndex+zIndex*xSeperation+yIndex*xSeperation*zSeperation;
					if(xIndex<0||yIndex<0|zIndex<0||xIndex>=xSeperation||yIndex>=ySeperation||zIndex>=zSeperation)
						continue;
					if(cells[index].lastPrimitiveIndex>=SOFMIS_SPATIALGRID_MAXPRIMITIVES){
						return;
					}

					cells[index].cellPrimitives[cells[index].lastPrimitiveIndex].index=p_primitiveIndex;
					cells[index].lastPrimitiveIndex++;
				}
	}

	inline void updateBounds(smSurfaceMesh* p_mesh,smInt p_index){

		//min
		aabb[p_index].aabbMin.x=  SOFMIS_MIN(p_mesh->vertices[p_mesh->triangles[p_index].vert[0]].x,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[1]].x);
		aabb[p_index].aabbMin.x = SOFMIS_MIN(aabb[p_index].aabbMin.x,
                                    p_mesh->vertices[p_mesh->triangles[p_index].vert[2]].x); 

		aabb[p_index].aabbMin.y=  SOFMIS_MIN(p_mesh->vertices[p_mesh->triangles[p_index].vert[0]].y,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[1]].y);
		aabb[p_index].aabbMin.y = SOFMIS_MIN(aabb[p_index].aabbMin.y,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]].y); 

		aabb[p_index].aabbMin.z=  SOFMIS_MIN(p_mesh->vertices[p_mesh->triangles[p_index].vert[0]].z,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[1]].z);
		aabb[p_index].aabbMin.z = SOFMIS_MIN(aabb[p_index].aabbMin.z,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]].z); 

		//max
		aabb[p_index].aabbMax.x=  SOFMIS_MAX(p_mesh->vertices[p_mesh->triangles[p_index].vert[0]].x,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[1]].x);
		aabb[p_index].aabbMax.x = SOFMIS_MAX(aabb[p_index].aabbMax.x,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]].x); 

		aabb[p_index].aabbMax.y=  SOFMIS_MAX(p_mesh->vertices[p_mesh->triangles[p_index].vert[0]].y,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[1]].y);
		aabb[p_index].aabbMax.y = SOFMIS_MAX(aabb[p_index].aabbMax.y,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]].y); 

		aabb[p_index].aabbMax.z=  SOFMIS_MAX(p_mesh->vertices[p_mesh->triangles[p_index].vert[0]].z,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[1]].z);
		aabb[p_index].aabbMax.z = SOFMIS_MAX(aabb[p_index].aabbMax.z,
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]].z); 
	}


	void updateBounds(){
		smInt numberOfprimitives;
		for(smInt i=0;i<mesh->nbrTriangles;i++)
			updateBounds(mesh,i);
	}

	void linkPrims(){
		for(smInt i=0;i<mesh->nbrTriangles;i++)
			linkPrimitivetoCell(i);
	}

	void addObject(smSceneObject *obj){
		smClassType objectType;
		linkedObject=obj->getObjectUnifiedID();
		objectType=obj->getType();

		switch(objectType){
			case SOFMIS_SMSTATICSCENEOBJECT:
				mesh=((smStaticSceneObject*)obj)->mesh;
			break;
		}
	}

	void draw(smDrawParam p_params){

		int temp;
		int index=0;
		int index2=0;
		smInt latticeMode;
		latticeMode=SOFMIS_SMLATTICE_CELLPOINTSLINKS;

		if(cells==NULL||latticeMode==SOFMIS_SMLATTICE_NONE)
			return;

		glMatrixMode(GL_MODELVIEW);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,(GLfloat*)&smColor::colorYellow );

		if(latticeMode&SOFMIS_SMLATTICE_SEPERATIONLINES)
		{	for(int j=0;j<ySeperation;j++){
			glDisable(GL_LIGHTING);
			glColor3fv((GLfloat*)&smColor::colorWhite);

			glBegin(GL_LINES);
				for(int i=0;i<xSeperation;i++){
					index=i+j*xSeperation*zSeperation;
					index2=index+xSeperation*(zSeperation-1);
					glVertex3f( cells[index].cellLeftCorner[0],
                                cells[index].cellLeftCorner[1],
                                cells[index].cellLeftCorner[2]-4*zStep);
					glVertex3f(cells[index2].cellLeftCorner[0],
                        cells[index2].cellLeftCorner[1],
                        cells[index2].cellLeftCorner[2]+4*zStep);
				}

				for(int i=0;i<zSeperation;i++){
					index=i*xSeperation+j*xSeperation*zSeperation;
					index2=index+(xSeperation-1);
					glVertex3f( cells[index].cellLeftCorner[0]-4*xStep,
                                cells[index].cellLeftCorner[1],
                                cells[index].cellLeftCorner[2]);
					glVertex3f(cells[index2].cellLeftCorner[0]+4*xStep,
                                cells[index2].cellLeftCorner[1],
                                cells[index2].cellLeftCorner[2]);
				}
			glEnd();
		}
		glEnable(GL_LIGHTING);
		glPopMatrix();
		}

		if(latticeMode&SOFMIS_SMLATTICE_CELLPOINTS||SOFMIS_SMLATTICE_CELLPOINTSLINKS){
			for(int y=0;y<ySeperation;y++)
				for(int z=0;z<zSeperation;z++)
					for(int x=0;x<xSeperation;x++){

						index=x+z*xSeperation+y*xSeperation*zSeperation;

						if(latticeMode&SOFMIS_SMLATTICE_CELLPOINTSLINKS){
							glDisable(GL_LIGHTING);
							glDisable(GL_TEXTURE_2D);

							glEnable(GL_COLOR_MATERIAL);

							glBegin(GL_LINE_STRIP);
								glColor3fv((GLfloat*)&smColor::colorWhite);
								glVertex3fv((GLfloat*)&cells[index].cellLeftCorner);
								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                            cells[index].cellLeftCorner[1],
                                            cells[index].cellLeftCorner[2]);
								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                            cells[index].cellLeftCorner[1],
                                            cells[index].cellLeftCorner[2]+zStep);
								glVertex3f(cells[index].cellLeftCorner[0],
                                            cells[index].cellLeftCorner[1],
                                            cells[index].cellLeftCorner[2]+zStep);
								glVertex3fv((GLfloat*)&cells[index].cellLeftCorner);

								glVertex3f(cells[index].cellLeftCorner[0],
                                            cells[index].cellLeftCorner[1]+yStep,
                                            cells[index].cellLeftCorner[2]);
								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                            cells[index].cellLeftCorner[1]+yStep,
                                            cells[index].cellLeftCorner[2]);
								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                            cells[index].cellLeftCorner[1]+yStep,
                                            cells[index].cellLeftCorner[2]+zStep);
								glVertex3f(cells[index].cellLeftCorner[0],
                                            cells[index].cellLeftCorner[1]+yStep,
                                            cells[index].cellLeftCorner[2]+zStep);
								glVertex3f(cells[index].cellLeftCorner[0],
                                            cells[index].cellLeftCorner[1]+yStep,
                                            cells[index].cellLeftCorner[2]);
							glEnd();

							glBegin(GL_LINES);
								glColor3fv((GLfloat*)&smColor::colorWhite);
								glVertex3fv((GLfloat*)&cells[index].cellLeftCorner);
								glVertex3f(cells[index].cellLeftCorner[0],
                                            cells[index].cellLeftCorner[1]+yStep,
                                            cells[index].cellLeftCorner[2]);

								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                    cells[index].cellLeftCorner[1],
                                    cells[index].cellLeftCorner[2]);
								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                    cells[index].cellLeftCorner[1]+yStep,
                                    cells[index].cellLeftCorner[2]);

								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                    cells[index].cellLeftCorner[1],
                                    cells[index].cellLeftCorner[2]+zStep);
								glVertex3f(cells[index].cellLeftCorner[0]+xStep,
                                    cells[index].cellLeftCorner[1]+yStep,
                                    cells[index].cellLeftCorner[2]+zStep);

								glVertex3f(cells[index].cellLeftCorner[0],
                                    cells[index].cellLeftCorner[1],
                                    cells[index].cellLeftCorner[2]+zStep);
								glVertex3f(cells[index].cellLeftCorner[0],
                                    cells[index].cellLeftCorner[1]+yStep,
                                    cells[index].cellLeftCorner[2]+zStep);
							glEnd();

							glEnable(GL_LIGHTING);
						}
					}
		}

		if(latticeMode&SOFMIS_SMLATTICE_MINMAXPOINTS){
			glPushMatrix();
				glPushMatrix();
					glTranslatef(cells[0].cellLeftCorner[0],cells[0].cellLeftCorner[1],cells[0].cellLeftCorner[2]);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,(GLfloat*)&smColor::colorYellow);
					glutSolidSphere(2,20,20);
				glPopMatrix();

				glPushMatrix();
					glTranslatef(cells[this->totalCells-1].cellRightCorner[0],
                                 cells[this->totalCells-1].cellRightCorner[1],
                                 cells[this->totalCells-1].cellRightCorner[2]);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,(GLfloat*)&smColor::colorRed );
					glutSolidSphere(2,20,20);
				glPopMatrix();
			glPopMatrix();
		}
	}

};

#endif
