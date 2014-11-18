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
    \brief	    This Module is core Module which should be derived by all module implementations.
    

*****************************************************
*/
#ifndef SMMODULE_H
#define SMMODULE_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smDispatcher.h"
#include "smCore/smScene.h"
#include "smCore/smSceneObject.h"
#include "smCore/smEventHandler.h"


///this class is module major. Every other thread should derive this class
class smModule:public smCoreClass{
	
private:    
   
	 ///each module has unique id
	 //smUnifiedID moduleId;
	 ///this is auto increment for module ids
	 //static QAtomicInt moduleIdCounter;

	 friend class smSDK;

protected:
	 ///initialization flag
	 smBool isInitialized;







protected:
	///execution termination..if it is true exit from the thread
	smBool terminateExecution;
	///When the terminatation is done by the module, this will be true
	smBool terminationCompleted;

	///scene list in the environment
	vector<smScene*>sceneList;
	
	///scene object lists for each scene 
    //vector<vector<smSceneObject*>>sceneObjectList;

    virtual void beginModule();
    virtual void   endModule();
    smDispatcher *dispathcer;
	smEventDispatcher *eventDispatcher;
	

		


    public:
        
    ///Abstract Constructor ever
    //virtual  smModule(smErrorLog *log=NULL);
		smModule(){
			//moduleId=moduleIdCounter.fetchAndAddOrdered(1);
			terminateExecution=false;
			eventDispatcher=NULL;
			isInitialized=false;
			name="Module";//+moduleId;
			
			


		
		}

     ///list the scenes and scene objects
     void list(){
        /* for(smInt m=0;m<sceneList.size();m++){
             cout<<"SCENE="<<sceneList[m]<<sceneList[m]->getSceneId() <<endl;
			 for(smInt n=0;n<sceneList[m]->sceneObjects.size();n++)
				 cout<<"\tOBJECTS"<<(sceneList[m])->sceneObjects[n]<<endl;
         }*/
     } 

	 virtual void init()=0;
	 virtual void beginFrame()=0;
	 virtual void endFrame()=0;
	 virtual void exec()=0;
	 void terminate(){
		 terminateExecution=true;
		
	 }
	 smBool isTerminationDone(){
		 return terminationCompleted;
	 
	 }
	 
	 void waitTermination(){
		 while(1){
			 if(terminationCompleted==true)
				 break;
		 }
	 
	 }



	 void setEventDispatcher(smEventDispatcher *p_dispathcer){
		  eventDispatcher=p_dispathcer;
	 
	 }
	 inline smInt getModuleId(){
		 return uniqueId.ID;
	 }
	 

	 




};





#endif