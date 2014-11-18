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
	//smVec3 *X;
	smVec3<smFloat> *P;
	smVec3<smFloat>  *V;
	smVec3<smFloat>  *exF;
	
	smInt nbrSpr;
	//smInt **sprIdx;
	smFloat *L0;

	smBool *fixedMass;


	smInt nbrFixedMass;
	smInt *listFixedMass;




	////////////////////////////////////////
	//ball
	smVec3<smFloat> ball_pos;
	smVec3<smFloat> ball_vel;
	smVec3<smFloat> ball_frc;
	
	smFloat ball_mass;
	smFloat ball_rad;



    
    smPBDSceneObject(smErrorLog *p_log=NULL){
        type=SOFMIS_SMPBDSCENEOBJECT;
       
		
		
    
    }

	///not implemented yet.
    virtual smSceneObject*clone()
    {
		return this;
    
    }


	//not implemented yet..tansel
	virtual void serialize(void *p_memoryBlock){
		

		
	
	}
	//not implemented yet..tansel
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

	///not implemented yet.
    virtual smSceneObject*clone()
    {
		return this;
    
    }


	//not implemented yet..tansel
	virtual void serialize(void *p_memoryBlock){
		smInt offset=0;
		smChar *memoryBlock=(smChar*)p_memoryBlock;
		memcpy(memoryBlock,mesh->vertices,sizeof(mesh->vertices));
		memoryBlock+=sizeof(sizeof(mesh->vertices));
		memcpy(memoryBlock,mesh->triangles,sizeof(smTriangle)*mesh->nbrTriangles);

		
	
	}
	//not implemented yet..tansel
	virtual void unSerialize(void *p_memoryBlock){
	
	}
	void initMeshStructure()
	{

		
		//X = 0;
		P = 0;
		V = 0;
		exF = 0;

		//sprIdx = 0;
		L0 = 0;

		fixedMass = 0;

		//triVertIdx = 0;
		//sprInTris = 0;

		listFixedMass = 0;
			

		paraK = 0.9;
		dT = 0.1;
		//dT = 0.001;//for debug purposes
		Damp = 0.0;

		
		
		////ball
		//ball_pos.setValue(0.0, 10.0, 0.0);
		//ball_vel.setValue(0.0, 0.0, 0.0);
		//ball_frc.setValue(0.0, 0.0, 0.0);

		//ball_mass = 1.0;
		//ball_rad = 1.0;
	}

	


	void InitSurfaceObject()
	{
		int i, j, k;
		//surface mesh
		nbrMass = mesh->nbrVertices;
		
		//X = new smVec3[nbrMass];
		P = new smVec3<smFloat>[nbrMass];
		V = new smVec3<smFloat>[nbrMass];
		exF = new smVec3<smFloat>[nbrMass];
		fixedMass = new bool[nbrMass];
		for(i=0;i<nbrMass;i++){	fixedMass[i] = false;}

		//findFixedMass();

		
		
	

		
		

		for(i=0;i<nbrMass;i++){
			P[i] = mesh->vertices[i];
		}

		nbrSpr = mesh->edges.size();
		L0 = new float[nbrSpr];
		for(i=0;i<nbrSpr;i++){
			//L0[i] = (X[sprIdx[i][0]] - X[sprIdx[i][1]]). module();		
			L0[i] = (mesh->vertices[mesh->edges[i].vert[0]] - mesh->vertices[mesh->edges[i].vert[1]]). module();		
		}
		mesh->allocateAABBTris();
	}


	~smPBDSurfaceSceneObject(){
		//if(X != 0) delete [] X;
		
		if(P != 0) delete [] P;
		if(V != 0) delete [] V;
		if(exF != 0) delete [] exF;

		if(L0 != 0) delete [] L0;

		if(fixedMass != 0) delete [] fixedMass;

	
	}

	void findFixedMassWrtSphere(smVec3f p_center, smFloat pos);
	void findFixedCorners()
	{
		nbrFixedMass = 2;
		listFixedMass = new smInt[nbrFixedMass];

		//temporary code/////////////////////////////////////////////////////////////////////////////////////////
		smVec3<smFloat> corner[2];
		smInt i, j;
		smFloat minmin, dist;

		/*corner[0].setValue(mesh->aabb.aabbMax.x, mesh->aabb.aabbMax.y, mesh->aabb.aabbMax.z);
		corner[1].setValue(mesh->aabb.aabbMin.x, mesh->aabb.aabbMax.y, mesh->aabb.aabbMax.z);
		corner[2].setValue(mesh->aabb.aabbMax.x, mesh->aabb.aabbMax.y, mesh->aabb.aabbMin.z);
		corner[3].setValue(mesh->aabb.aabbMin.x, mesh->aabb.aabbMax.y, mesh->aabb.aabbMin.z);*/
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
		if(listFixedMass != 0) delete [] listFixedMass;
		//temporary code/////////////////////////////////////////////////////////////////////////////////////////
	}
	
	 virtual void draw(smDrawParam p_params);


};



#endif