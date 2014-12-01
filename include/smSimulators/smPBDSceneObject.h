/*
****************************************************
                  SIMMEDTK LICENSE
****************************************************
*/

#ifndef SMPBDSCENEOBJECT_H
#define SMPBDSCENEOBJECT_H
#include "smCore/smConfig.h"
#include "smCore/smSceneObject.h"
#include "smMesh/smSurfaceMesh.h"
#include "smMesh/smMesh.h"

class smPBDSceneObject:public smSceneObject {
public:
	smFloat dT;
	smFloat paraK;
	smFloat Damp;
	smInt nbrMass;
	smInt **massIdx;
	smVec3<smFloat> *P;
	smVec3<smFloat>  *V;
	smVec3<smFloat>  *exF;
	smInt nbrSpr;
	smFloat *L0;
	smBool *fixedMass;
	smInt nbrFixedMass;
	smInt *listFixedMass;

	smVec3<smFloat> ball_pos;
	smVec3<smFloat> ball_vel;
	smVec3<smFloat> ball_frc;

	smFloat ball_mass;
	smFloat ball_rad;

	smPBDSceneObject(smErrorLog *p_log=NULL){
		type=SOFMIS_SMPBDSCENEOBJECT;
	}

	virtual smSceneObject*clone()
	{
		return this;
	}

	virtual void serialize(void *p_memoryBlock){

	}

	virtual void unSerialize(void *p_memoryBlock){

	}

	void findFixedMass();
};

class smPBDSurfaceSceneObject:public smPBDSceneObject {
public:
	smSurfaceMesh *mesh;

	smInt nbrTri;
	smInt **triVertIdx;
	smInt **sprInTris;		//which triangles include a spring


	smPBDSurfaceSceneObject(smErrorLog *p_log=NULL){
		type=SOFMIS_SMPBDSURFACESCENEOBJECT;
		mesh=new smSurfaceMesh(SMMESH_DEFORMABLE,p_log);
	}

	virtual smSceneObject*clone()
	{
		return this;
	}

	virtual void serialize(void *p_memoryBlock){
		smInt offset=0;
		smChar *memoryBlock=(smChar*)p_memoryBlock;
		memcpy(memoryBlock,mesh->vertices,sizeof(mesh->vertices));
		memoryBlock+=sizeof(sizeof(mesh->vertices));
		memcpy(memoryBlock,mesh->triangles,sizeof(smTriangle)*mesh->nbrTriangles);
	}

	virtual void unSerialize(void *p_memoryBlock){

	}

	void initMeshStructure()
	{
		P = 0;
		V = 0;
		exF = 0;
		L0 = 0;
		fixedMass = 0;
		listFixedMass = 0;
		paraK = 0.9;
		dT = 0.1;
		Damp = 0.0;
	}

	void InitSurfaceObject()
	{
		int i, j, k;
		//surface mesh
		nbrMass = mesh->nbrVertices;

		P = new smVec3<smFloat>[nbrMass];
		V = new smVec3<smFloat>[nbrMass];
		exF = new smVec3<smFloat>[nbrMass];
		fixedMass = new bool[nbrMass];
		for(i=0;i<nbrMass;i++){
			fixedMass[i] = false;
		}

		for(i=0;i<nbrMass;i++){
			P[i] = mesh->vertices[i];
		}

		nbrSpr = mesh->edges.size();
		L0 = new float[nbrSpr];

		for(i=0;i<nbrSpr;i++){
			L0[i] = (mesh->vertices[mesh->edges[i].vert[0]] - mesh->vertices[mesh->edges[i].vert[1]]). module();
		}

		mesh->allocateAABBTris();
	}


	~smPBDSurfaceSceneObject(){
		if(P != 0)
			delete [] P;
		if(V != 0)
			delete [] V;
		if(exF != 0)
			delete [] exF;
		if(L0 != 0)
			delete [] L0;
		if(fixedMass != 0)
			delete [] fixedMass;
	}

	void findFixedMassWrtSphere(smVec3f p_center, smFloat pos);

	void findFixedCorners(){
		nbrFixedMass = 2;
		listFixedMass = new smInt[nbrFixedMass];
		smVec3<smFloat> corner[2];
		smInt i, j;
		smFloat minmin, dist;
		corner[0].setValue(mesh->aabb.aabbMax.x, mesh->aabb.aabbMax.y, mesh->aabb.aabbMax.z);
		corner[1].setValue(mesh->aabb.aabbMin.x, mesh->aabb.aabbMax.y, mesh->aabb.aabbMax.z);
		listFixedMass[0]=-1;
		listFixedMass[1]=-1;

		for(i=0;i<nbrFixedMass;i++){
			minmin = smMAXFLOAT;
			for(j=0;j<mesh->nbrVertices;j++){
				dist = (corner[i] - mesh->vertices[j]).module();
				if(dist<minmin){
					minmin = dist;
					listFixedMass[i] = j;
				}
			}
		}

		for(i=0;i<nbrFixedMass;i++){
			fixedMass[listFixedMass[i]] = true;
		}
		if(listFixedMass != 0)
			delete [] listFixedMass;
	}

	virtual void draw(smDrawParam p_params);
};

#endif
