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

#ifndef SMSTATICSCENEOBJECT_H
#define SMSTATICSCENEOBJECT_H
#include "smCore/smConfig.h"
#include "smCore/smSceneObject.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smCoreClass.h"



class smStaticSceneObject:public smSceneObject {
 public: 
    smSurfaceMesh *mesh;


    
    smStaticSceneObject(smErrorLog *p_log=NULL){
        type=SOFMIS_SMSTATICSCENEOBJECT;
		mesh=new smSurfaceMesh(SMMESH_RIGID,p_log);
		
		
    
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
	virtual void draw(smDrawParam p_params);

	
	


};



#endif