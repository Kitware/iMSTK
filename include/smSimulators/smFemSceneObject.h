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
\brief	    This Class is an example static object scene. this doesn't have any dynamics. It is for only rendering the static mesh in the scene  

*****************************************************
*/

#ifndef SMFEMSCENEOBJECT_H
#define SMFEMSCENEOBJECT_H

#include <Eigen/Core>
#include <Eigen/LU>
#include <QAtomicInt>
#include "smCore/smConfig.h"
#include "smMesh/smVolumeMesh.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smRendering/smConfigRendering.h"
#include "smRendering/smCustomRenderer.h"
#include "smCore/smMemoryBlock.h"
using namespace Eigen;



class smFemSceneObject:public smSceneObject {
public: 
	//fem objetc has two representations: surface and volume	
	smVolumeMesh *v_mesh;

	MatrixXf LM;
	MatrixXf ID;	

	smInt totalDof;

	smFloat dT;
	smFloat density;

	smBool dynamicFem;
	smBool pullUp;

	MatrixXf stiffnessMatrix;
	MatrixXf Kinv;	
	Matrix3f k33;
	Matrix3f k33inv;

	VectorXf displacements;
	VectorXf temp_displacements;
	VectorXf displacements_prev;
	VectorXf dymamic_temp;
	VectorXf componentMasses;
	VectorXf fm;
	VectorXf fm_temp;
	VectorXf totalDisp;
	VectorXf nodeMass;
	VectorXf viscosity;

	smInt pulledNode;


	smFemSceneObject(smErrorLog *p_log=NULL){

		type=SOFMIS_SMFEMSCENEOBJECT;		
		v_mesh=new smVolumeMesh(SMMESH_DEFORMABLE,p_log);
		pullUp=true;
		dynamicFem=false;
		if(dynamicFem){
			dT=0.02;
			density=500;
		}
	}

	///not implemented yet.
	virtual smSceneObject*clone()
	{
		return this;

	}

	void buildLMmatrix();
	void computeStiffness();
	smBool loadMatrix(const smChar *fname,MatrixXf &a);
	smBool saveMatrix(const smChar *fname,MatrixXf &a);
	smFloat V(smInt xyz,smInt xyz123, smInt tet);
	void assembleK(smInt element, MatrixXf k);
	void calculateDisplacements_QStatic(smVec3<smFloat>*);
	void calculateDisplacements_Dynamic(smVec3<smFloat> *vertices);
	void lumpMasses();
	smFloat tetraVolume(Vector3f &a,Vector3f &b,Vector3f &c,Vector3f &d);
	


	//not implemented yet..tansel
	virtual void serialize(void *p_memoryBlock){
		smInt offset=0;
		smChar *memoryBlock=(smChar*)p_memoryBlock;
		memcpy(memoryBlock,v_mesh->vertices,sizeof(v_mesh->vertices));
		memoryBlock+=sizeof(sizeof(v_mesh->vertices));
		memcpy(memoryBlock,v_mesh->triangles,sizeof(smTriangle)*v_mesh->nbrTriangles);



	}
	//not implemented yet..tansel
	virtual void unSerialize(void *p_memoryBlock){

	}
	virtual void draw(smDrawParam p_params);





};



#endif