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
    \brief	    This Module is for scene object. Every object implementation should derive this class and customize it

*****************************************************
*/
#ifndef SMSCENEOBJECT_H
#define SMSCENEOBJECT_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smRendering/smConfigRendering.h"
#include "smRendering/smCustomRenderer.h"
#include "smCore/smMemoryBlock.h"
#include <QAtomicInt>

//forward class declaration
class smObjectSimulator;
class smCustomRenderer;



///booleans for objects indicate whether they're initialized or not.
struct smObjectInitFlags{
	smBool isViewerInit;
	smBool isSimulatorInit;

};

class smSceneObject:public smCoreClass{
     smObjectSimulator    *objectSim;  
     smCustomRenderer    *customRender;

	 friend class smSDK;
	 friend class smViewer;
	 friend class smScene;
	 friend class smObjectSimulator;
	 
private:
///thread safe Id counter
	//static QAtomicInt sceneObjectIdCounter;

	///get unique Id
	/*static smInt getNewObjectId(){
		 smInt ret;
		 ret=sceneObjectIdCounter.fetchAndAddOrdered(1);
		
		 return ret;
	}*/
	
protected:
	///memory allocation class. which is accessed by mainly object simualtors
	

public:    
	///sceneId which is unique accross the whole objects within the SOFMIS
	//smUnifiedID sceneObjectId;
	


   
	smMemoryBlock *memBlock;
	smObjectInitFlags flags;
	
   
    smSceneObject();
	//{
 //       type=SOFMIS_SMSCENEBOJECT;       
 //       objectSim=NULL;
	//	//default rendering type	
	//	//renderDetail.renderType=SOFMIS_RENDER_MATERIALCOLOR;
	//	customRender=NULL;
	//	//sceneId which is unique accross the whole objects within the SOFMIS
	//	sceneObjectId.ID=getNewObjectId();
	//	smSDK::registerSceneObject(this);
	//	memBlock=new smMemoryBlock(NULL);
	//	 flags.isViewerInit=false;
	//	 flags.isSimulatorInit=false;
	//	 name="SceneObject"+sceneObjectId.ID;
	//	 
	//	
	//
 //   }
    

	smInt getObjectId(){
		return uniqueId.ID;
	}
	smUnifiedID getObjectUnifiedID(){
		 return	 uniqueId;
	}
   
	virtual void attachObjectSimulator(smObjectSimulator *p_objectSim);
	virtual void releaseObjectSimulator();
	smObjectSimulator* getObjectSimulator();
	void attachCustomRenderer(smCustomRenderer *p_customeRenderer);
	void releaseCustomeRenderer();
	
	///serialize function explicity writes the object to the memory block
	///each scene object should know how to write itself to a memory block
	virtual void serialize(void *p_memoryBlock)=0;
	
	///Unserialize function can recover the object from the memory location
	virtual void unSerialize(void *p_memoryBlock)=0;



	///this function may not be used
    ///every Scene Object should know how to clone itself. Since the data structures will be 
    ///in the beginning of the modules(such as simulator, viewer, collision etc.)
    virtual smSceneObject*clone()=0;


	//comment: make the this routine abstact so that enforce everybody to write their initiazaliton routines
	///the initialization routuines belong to the objects should be called herein
	virtual void init(){
	
	};




	
   



};


#endif
