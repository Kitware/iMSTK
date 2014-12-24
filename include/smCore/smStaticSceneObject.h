/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMSTATICSCENEOBJECT_H
#define SMSTATICSCENEOBJECT_H

#include "smCore/smConfig.h"
#include "smCore/smSceneObject.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smCoreClass.h"
/// \brief static scene object
class smStaticSceneObject:public smSceneObject {

public: 
	/// \brief static scene object contains a mesh
	smSurfaceMesh *mesh;
	/// \brief constructor receives the error log
	smStaticSceneObject(smErrorLog *p_log=NULL){
		type=SIMMEDTK_SMSTATICSCENEOBJECT;
		mesh=new smSurfaceMesh(SMMESH_RIGID,p_log);
	}

	///not implemented yet.
	virtual smSceneObject*clone(){
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
	/// \brief called if the object is added to the viewer.
	virtual void draw(smDrawParam p_params);
};

#endif
