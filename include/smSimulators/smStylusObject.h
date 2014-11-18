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
    \brief	    This Class is an example for stylus for different modalities
*****************************************************
*/

#ifndef SMSTYLUSOBJECT_H
#define SMSTYLUSOBJECT_H

#include <QVector>
#include <QStack>

#include "smCore/smConfig.h"
#include "smMesh/smMesh.h"
#include "smCore/smSceneObject.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMath.h"
#include "smExternal/tree.hh"



template<typename T> class smCollisionModel;
template<typename smSurfaceTreeCell> class smSurfaceTree;
struct smOctreeCell;








struct smMeshContainer{
	public:

	QString name;
	
	smMeshContainer(QString p_name=""){
		//offsetRot.setIdentity();
		name=p_name;
		offsetRotX=0.0;
		offsetRotY=0.0;
		offsetRotZ=0.0;
		preOffsetPos.setValue(0,0,0);
		posOffsetPos.setValue(0,0,0);
		//totalChildMeshes=0;
		mesh=NULL;
		colModel=NULL;

		
	}
	smMeshContainer(QString p_name,smMesh *p_mesh,smVec3<smFloat> p_prePos,smVec3<smFloat> p_posPos,smFloat p_offsetRotX,smFloat p_offsetRotY,smFloat p_offsetRotZ){
		offsetRotX=p_offsetRotX;
		offsetRotY=p_offsetRotY;
		offsetRotZ=p_offsetRotZ;
		preOffsetPos=p_prePos;
		posOffsetPos=p_posPos;
		name=p_name;
		
		colModel=NULL;
		//totalChildMeshes=0;
	}

	
	//smInt totalChildMeshes;
	smFloat offsetRotX;
	smFloat offsetRotY;
	smFloat offsetRotZ;
	smVec3<smDouble> preOffsetPos;
	smVec3<smDouble> posOffsetPos;
	smMatrix44<smDouble> accumulatedMatrix;
	smMatrix44<smDouble> accumulatedDeviceMatrix;

	smMatrix44<smDouble> currentMatrix;
	smMatrix44<smDouble> currentViewerMatrix;
	smMatrix44<smDouble> currentDeviceMatrix;
	smMatrix44<smDouble> tempCurrentMatrix;
	smMatrix44<smDouble> tempCurrentDeviceMatrix;
	smMesh * mesh;
	smSurfaceTree<smOctreeCell> *colModel;
	//smOctreeCollModel *colModel;

	

	//void computeCurrentMatrix();
	
	 inline void computeCurrentMatrix(){
		smMatrix33<smDouble> matX,matY,matZ;
		smMatrix33<smDouble> res;
		smMatrix44<smDouble> trans,trans1;
		smMatrix44<smDouble> temp;
		
		trans.setTranslation(preOffsetPos.x,preOffsetPos.y,preOffsetPos.z);
		matX.rotAroundX(SM_PI_TWO*offsetRotX);
		matY.rotAroundY(SM_PI_TWO*offsetRotY);
		matZ.rotAroundZ(SM_PI_TWO*offsetRotZ);
		trans1.setTranslation(posOffsetPos.x,posOffsetPos.y,posOffsetPos.z);

		res=matX*matY*matZ;
		tempCurrentMatrix=accumulatedMatrix*trans*res*trans1;
		tempCurrentDeviceMatrix=accumulatedDeviceMatrix*trans*res*trans1;
		//currentMatrix=temp;
	}
		



};



//struct smMeshIterator{
//	QStack<smMeshContainer*> stack;
//	QStack<smInt> stackIndexLevel;
//	smMeshContainer *rootContainer;
//	smMeshContainer *currentContainer;
//
//	smInt rootIndex;
//	smMeshIterator(smMeshContainer *p_container){
//			
//		rootContainer=p_container;
//		//indexLevel=0;
//		stack.push(rootContainer);
//		//
//
//
//		
//	}
//	void next(){
//		
//		while(true){
//			if(currentContainer->childs.size()>0)
//			{
//				stack.push(currentContainer);
//				//stackIndexLevel=indexLevel;
//
//				//currentContainer=currentContainer->childs[
//			}
//			else{
//				currentContainer=stack.pop();
//				//currentContainer->mesh
//				//indexLevel++;
//				
//			}
//		}
//
//		
//	}
//	
//
//};
//
struct smStylusPoints{

	smStylusPoints(){
		point.setValue(0,0,0);
		container=NULL;
	}
	smVec3 <smFloat> point;
	smMeshContainer *container;

};
				
class smStylusSceneObject:public smSceneObject{

public:

	smVec3 <smDouble> pos;
	smVec3 <smDouble> vel;
	smMatrix33 <smDouble> rot;
	smMatrix44 <smDouble> transRot;
	smMatrix44 <smDouble> transRotDevice;
	smBool toolEnabled;


	//QVector<smMeshContainer*> meshContainers;
    smStylusSceneObject(smErrorLog *p_log=NULL);
	virtual void serialize(void *p_memoryBlock){
	
	}
	virtual void unSerialize(void *p_memoryBlock){
	
	}

	virtual void handleEvent(smEvent *p_event){};

	
  


};


class smStylusRigidSceneObject:public smStylusSceneObject,public smEventHandler{
	 QHash<QString,tree<smMeshContainer*>::iterator> indexIterators;
public:
	tree<smMeshContainer*> meshes;
	tree<smMeshContainer*>::iterator rootIterator; 
	//smBool preTraverseCallbackEnabled;
	
	//void preTraverseCallBack(smMeshContainer &p_container);
	volatile smBool updateViewerMatrixEnabled;


	//to show the device tool..It is for debugging god object
	smBool enableDeviceManipulatedTool;

	
	///enabling post traverse callback enabled for each node.
	smBool posTraverseCallbackEnabled;
	///post traverse callback for each node
	virtual void posTraverseCallBack(smMeshContainer &p_container);
	
	///Post Traverse callback for the entire object.
	virtual void posTraverseCallBack(){}
	smBool posCallBackEnabledForEntireObject;
	
	
	smStylusRigidSceneObject(smErrorLog *p_log=NULL);

    tree<smMeshContainer*>::iterator &addMeshContainer(smMeshContainer *p_meshContainer){
		//meshContainers.push_back(p_meshContainer);
		 tree<smMeshContainer*>::iterator iter;
		 if(meshes.size()>1)
			 iter=meshes.append_child(rootIterator,p_meshContainer);
		    
		 else
		    iter=meshes.insert(rootIterator,p_meshContainer);
		 indexIterators.insert(p_meshContainer->name,iter);

	   
	   return iter;
		
   }

 smBool addMeshContainer(QString p_ParentName,smMeshContainer *p_meshContainer){
	 tree<smMeshContainer*>::iterator iter; 
	 if(p_ParentName.size()>0){
		 if(indexIterators.contains(p_ParentName)){
		   iter=indexIterators[p_ParentName];
		   meshes.append_child(iter,p_meshContainer);
			return true;
		 }
		 else
			return false;

	 }
	 else 
		 return false;
	 
	 
		
   }

  tree<smMeshContainer*>::iterator addMeshContainer(tree<smMeshContainer*>::iterator p_iterator,smMeshContainer *p_meshContainer){
		//meshContainers.push_back(p_meshContainer);
	   return meshes.insert(p_iterator,p_meshContainer);
   }

  smMeshContainer *getMeshContainer(QString p_string) const;
  

  virtual void handleEvent(smEvent *p_event){};



  ///not complete yet
  smSceneObject *clone(){
	 // smMeshContainer *tempMeshCont;
	  smStylusRigidSceneObject *ret=new smStylusRigidSceneObject();
	 // this->pos=pos;
	 // this->rot=rot;
	 // for(smInt i=0;i<meshContainers.size();i++){
		//  ret->meshContainers.push_back(meshContainers.at(i));
		//  ret->meshContainers.at(i).mesh=(smMesh*)meshContainers.at(i).mesh->clone();
		//  tempMeshCont=ret->meshContainers.at(i);
		//  for(smInt j=0;i<ret->meshContainers.at(i).totalChildMeshes;j++){
		//			  
		//  }

	 // }
	 // 
	  return ret;
  
  }

  

  virtual void initDraw(smDrawParam p_params);

  virtual void draw(smDrawParam p_params);

  //virtual void IsThisToolLeftOrRight(smInt deviceID, smInt taskID){};
  
};

//class smStylusDeformableSceneObject:public smStylusSceneObject,public smEventHandler{
class smStylusDeformableSceneObject:public smStylusSceneObject{
	smStylusDeformableSceneObject(smErrorLog *p_log=NULL);
	


};



#endif