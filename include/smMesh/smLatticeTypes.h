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
    \brief	    This header file contains the example lattice type implementations. The lattice
				type could be any type and could be extended to anything
*****************************************************
*/

#ifndef SMLATTICETYPES_H
#define SMLATTICETYPES_H
//#include "smConfig.h"
//#include "smCoreClass.h"
//#include "smVec3.h"
//#include "smCustomRenderer.h"
//#include "smSurfaceMesh.h"
//#include "smLattice.h"
//#include "smGeometry.h"
//#include "smStaticSceneObject.h"


//typedef smPrimitive<smSurfaceMesh> smPrimitiveMeshTriangle;




//class smTrianglePrimitive:public smPrimitiveMeshTriangle{
//public:
//	//later on also make aabbMin and aabbMax as template definitions
//	smAABB aabb;
//	smInt primIndex;
//
//	
//
//	
//	smTrianglePrimitive(){
//	    aabb.aabbMin.setValue(0,0,0);
//		aabb.aabbMax.setValue(0,0,0);
//	
//	}
//
//	/*inline smSurfaceMesh* getMesh(smSceneObject *p_object){
//		switch (p_object->getType()){
//		   case SOFMIS_SMSTATICSCENEOBJECT:
//				return ((smStaticSceneObject*)p_object)->mesh;
//			   break;
//
//		}
//		return NULL;
//	
//	}*/
//	static inline  int getNbrPrimitives(smSurfaceMesh *mesh){
//		return mesh->nbrTriangles;
//
//	
//	}
//
//	
//	inline virtual smBool checkBroaderPhaseCollision(smPrimitive &p_Primitive){
//		smTrianglePrimitive *trianglePrimitive;
//		trianglePrimitive=(smTrianglePrimitive *)&(p_Primitive);
//		return smAABB::checkOverlap(aabb,trianglePrimitive->aabb);
//	}
//
//	inline virtual smBool checkNarrowPhaseCollision(smPrimitive &p_Primitive){
//		smTrianglePrimitive *trianglePrimitive;
//		trianglePrimitive=(smTrianglePrimitive *)&(p_Primitive);		
//		return false;
//
//			
//	}
//	
//	
//	
//	
//	/*inline  void updateBounds(smSurfaceMesh *mesh,smTrianglePrimitive *primitives){
//		
//		#define SOFMIS_PARALEL_FOR
//		for(smInt i=0;i<mesh->nbrTriangles;i++){
//			 updateBounds(mesh,i);
//		
//		}
//	}*/  
//	
//	
//};
//
///*
//class smTrianglePrimConnector:public smPrimConnector<smTrianglePrimitive,smSurfaceMesh>{
//	
//	smTrianglePrimConnector(smSurfaceMesh *p_mesh){
//		mesh=p_mesh;
//		
//		
//	
//	}
//	void initPrimitives(){
//		primitives=new smTrianglePrimitive[mesh->nbrTriangles];
//		nbrPrimitives=mesh->nbrTriangles;
//
//	}
//	void reservePrimitives(smInt p_totalPrims){
//		primitives=new smTrianglePrimitive[p_totalPrims];
//		nbrPrimitives=0;
//
//
//	}
//	inline void updateBounds(){
//		nbrPrimitives=mesh->nbrTriangles;
//		for(smInt i=0;i<nbrPrimitives;i++){
//			primitives[i].updateBounds(mesh,i);
//		}
//	}
//	///it is for parallel routines
//	inline void updateBoundPrim(smInt index){
//		primitives[index].updateBounds(mesh,index);
//		
//	}
//
//
//	~smTrianglePrimConnector(){
//		delete []primitives;
//	
//	}
//
//
//
//};
//
//*/ 

#endif

















