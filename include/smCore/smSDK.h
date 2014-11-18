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
    \brief	    This Module is for SDK core.This class is resposible for major module creation and updates the intenral pointers

*****************************************************
*/

#ifndef SMSDK_H
#define SMSDK_H
#include <QHash>
#include <QVector>
#include <QApplication>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smScene.h"
#include "smRendering/smViewer.h"
#include "smCore/smDispatcher.h"
#include "smCore/smSimulator.h"
#include "smCore/smTextureManager.h"
#include "smCore/smEventHandler.h"
#include "smCore/smModule.h"
#include "smUtilities/smDataStructs.h"
//#include "smDoubleBuffer.h"

#define SOFMIS_SDK_MAXMESHES 100
#define SOFMIS_SDK_MAXMODULES 100
#define SOFMIS_SDK_MAXOBJECTSIMULATORS 100
#define SOFMIS_SDK_MAXSCENES 100
#define SOFMIS_SDK_MAXSCENEOBJTECTS 100

class smMotionTransformer;
class smPipe;
template<typename T> class smIndiceArray;
class smScene;

enum smSDKReturnType{
	SOFMIS_SDK_MODULEREGISTERED,
	SOFMIS_SDK_MODULEREGISTEREDALREADY


};

struct smBaseHolder{
 /* smBool isActive;
  smBool kill;
  smBaseHolder(){
	isActive=true;
	kill=false;
  }*/
};

struct smMeshHolder:public smBaseHolder{
	smMeshHolder(){
		mesh=NULL;
	}
	smBaseMesh* mesh;
	inline smBool operator ==(smMeshHolder &p_param){
		return mesh==p_param.mesh;
	
	}
};
struct smModuleHolder:public smBaseHolder{
	smModuleHolder(){
		module=NULL;
	}
	smModule* module;
	inline smBool operator ==(smModuleHolder &p_param){
		return module==p_param.module;
	
	}
};
struct smObjectSimulatorHolder:public smBaseHolder{
	smObjectSimulatorHolder(){
		objectSim=NULL;
	}
	smObjectSimulator* objectSim;
	inline smBool operator ==(smObjectSimulatorHolder &p_param){
		return objectSim==p_param.objectSim;
	
	}
};
struct smSceneHolder:public smBaseHolder{
	smSceneHolder(){
		scene=NULL;
	
	}
	smScene* scene;
	inline smBool operator ==(smSceneHolder &p_param){
		return scene==p_param.scene;
	
	}
};
struct smSceneObjectHolder:public smBaseHolder{
	smSceneObjectHolder(){
		sceneObject=NULL;
	
	}
	smSceneObject* sceneObject;
	inline  smBool operator ==(smSceneObjectHolder &p_param){
		return sceneObject==p_param.sceneObject;
	
	}
};

struct smPipeHolder:public smBaseHolder{
	smPipe *pipe;
	smPipeHolder(){
	
		pipe=NULL;
	}
	inline  smBool operator ==(smPipeHolder &p_param){
		return pipe==p_param.pipe;
	
	}
	inline friend smBool operator==(smPipeHolder &p_pipe,QString &p_name){
		return (*(p_pipe.pipe)==p_name);
	}


};


class smSDK:public smCoreClass,public smEventHandler{
    protected:
		///this id is incremented automatically when the scene is created.
		smInt  sceneIdCounter;
		//smInt objectIdCounter;

        QApplication *application;
        smViewer *viewer;
		smSimulator *simulator;
		


        vector<smScene*>sceneList;
       // QMutex sceneMutex;
        //QMutex moduleMutex;
        static smErrorLog *errorLog;
        smDispatcher *dispathcer;
		smEventDispatcher *eventDispatcher;
        smInt argc;
        smChar argv;
		///modules are registered and stored 
		//QHash<smInt,smModule*> modules;
		//QHash<QString,smModule*> modulesByName;

		smBool isModulesStarted;
		

		void updateSceneListAll();
		void initRegisteredModules();
		void runRegisteredModules();
		
		static smSDK sdk;

		static smIndiceArray<smMeshHolder> *meshesRef;
		static smIndiceArray<smModuleHolder> *modulesRef;
		static smIndiceArray<smObjectSimulatorHolder> *objectSimulatorsRef;
		static smIndiceArray<smObjectSimulatorHolder> *collisionDetectorsRef;
		static smIndiceArray<smSceneHolder> *scenesRef;
		static smIndiceArray<smSceneObjectHolder> *sceneObjectsRef;
		static smIndiceArray<smMotionTransformer *> *motionTransRef;
		static smIndiceArray<smPipeHolder>* pipesRef;

		~smSDK();

		 smSDK(){
			
            smInt argc=1;
            smChar *argv[]={SOFMISVERSION_TEXT};
            application=new QApplication(argc,argv);
            errorLog=new smErrorLog();
            dispathcer=new smDispatcher();
			eventDispatcher= new smEventDispatcher();
			eventDispatcher->registerEventHandler(this,SOFMIS_EVENTTYPE_KEYBOARD);

			sceneIdCounter=1;
			//objectIdCounter=1;
			isModulesStarted=false;
	        type=SOFMIS_SMSDK;
            viewer=NULL;
			simulator=NULL;
            sceneList.clear();


			meshesRef=new smIndiceArray<smMeshHolder>(SOFMIS_SDK_MAXMESHES);
			modulesRef=new smIndiceArray<smModuleHolder>(SOFMIS_SDK_MAXMODULES) ;
			objectSimulatorsRef=new smIndiceArray<smObjectSimulatorHolder>(SOFMIS_SDK_MAXOBJECTSIMULATORS);
			collisionDetectorsRef=new smIndiceArray<smObjectSimulatorHolder>(SOFMIS_SDK_MAXOBJECTSIMULATORS) ;
			scenesRef=new smIndiceArray<smSceneHolder>(SOFMIS_SDK_MAXSCENES);
			sceneObjectsRef=new smIndiceArray<smSceneObjectHolder>(SOFMIS_SDK_MAXSCENEOBJTECTS);
			pipesRef=new smIndiceArray<smPipeHolder>(SOFMIS_SDK_MAXSCENEOBJTECTS);
        }
		

    public:
		//shutdowns all the modules
		void shutDown();


		//for now both functions below are the same. But it maybe subject to change.
		static smSDK * createSDK(){
			return &sdk;
		}
		static smSDK *getInstance(){
			return &sdk;
		
		}
		///SDK creates viewer
        smViewer *createViewer();
		///SDK creates simualtor
		smSimulator *createSimulator();
		///SDK creates scene
        smScene  *createScene();
		///SDK returns logger for the system
		static smErrorLog *getErrorLog(){
			return errorLog;
		};


		 /*smSDKReturnType registerModule(smModule *module){
			smInt moduleId;
			moduleId=module->getModuleId();
			//moduleMutex.lock();
				if(modules.contains(moduleId))
					return SOFMIS_SDK_MODULEREGISTERED;
				else{
					modules[moduleId]=module;
					return SOFMIS_SDK_MODULEREGISTERED;
					
				}
			//moduleMutex.unlock();
		}  */

		///SDK returns the event dispather
		inline smEventDispatcher *getEventDispatcher(){
			return eventDispatcher;
		}

		

		///terminates every module. Do it later on with smMessager
		void terminateAll(){
			/*
			QHash<smInt,smModule*>::iterator moduleIterator=modules.begin();
				
			while(moduleIterator!=modules.end()){
				(moduleIterator.value())->terminateExecution=true;
				 moduleIterator++;
		
			}
			while(moduleIterator!=modules.end()){
				(moduleIterator.value())->waitTermination();
				 moduleIterator++;
		
			}
			*/
				  
			/*
				viewer->terminateExecution=true;
				simulator->terminateExecution=true;
				simulator->waitTermination();
			*/
			
			for(smInt i=0;i<(*modulesRef).size();i++)
				(*modulesRef)[i].module->terminateExecution=true;

			
			for(smInt i=0;i<(*modulesRef).size();i++){
				smBool moduleTerminated=false;
				while(true&&!moduleTerminated){
					if((*modulesRef)[i].module->isTerminationDone()){
					   moduleTerminated=true; 
					}
				}

			}
			

			
			
		}

		 ///release the scene from the SDK..not implemented yet
        void releaseScene(smScene*);
        void run();
		static void addRef(smCoreClass* p_coreClass);
		static void removeRef(smCoreClass* p_coreClass);
		
		static smInt registerMesh(smBaseMesh*p_mesh){
			smMeshHolder mh;
			mh.mesh=p_mesh;
			return (p_mesh->uniqueId.sdkID=meshesRef->checkAndAdd(mh));
		} 
		static smInt registerModule(smModule *p_mod){
			smModuleHolder mh;
			mh.module=p_mod;
			return (p_mod->uniqueId.sdkID=modulesRef->checkAndAdd(mh));
		} 
		static void registerObjectSim(smObjectSimulator*p_os){
			smObjectSimulatorHolder os;
			os.objectSim=p_os;
			p_os->uniqueId.sdkID=objectSimulatorsRef->checkAndAdd(os);
		} 
		static void registerCollDet(smObjectSimulator*p_col){
			smObjectSimulatorHolder col;
			col.objectSim=p_col;
			p_col->uniqueId.sdkID=collisionDetectorsRef->checkAndAdd(col);
			
		} 
		static void registerScene(smScene *p_sc){
			smSceneHolder sc;
			sc.scene=p_sc;
			p_sc->uniqueId.sdkID=scenesRef->checkAndAdd(sc);
		} 
		static smBool unRegisterScene(smScene *p_sc){
			return scenesRef->remove(p_sc->uniqueId.sdkID);
		}
		
		static void registerSceneObject(smSceneObject*p_sco){
			smSceneObjectHolder  sh;
			sh.sceneObject=p_sco;
			p_sco->uniqueId.sdkID=sceneObjectsRef->checkAndAdd(sh);
		} 

		
		inline static smBaseMesh* getMesh(smUnifiedID &p_unifiedID){
			return (meshesRef->getByRef(p_unifiedID.sdkID).mesh);
		}
		inline static smModule* getModule(smUnifiedID &p_unifiedID){
			return (modulesRef->getByRef(p_unifiedID.sdkID).module);
		}
		inline static smObjectSimulator* getObjectSim(smUnifiedID &p_unifiedID){
			return (objectSimulatorsRef->getByRef(p_unifiedID.sdkID).objectSim);
		}
		inline static smObjectSimulator* getCollDet(smUnifiedID &p_unifiedID){
			return (collisionDetectorsRef->getByRef(p_unifiedID.sdkID).objectSim);
		}
		inline static smScene* getScene(smUnifiedID &p_unifiedID){
			return (scenesRef->getByRef(p_unifiedID.sdkID).scene);
		}
		inline static smSceneObject* getSceneObject(smUnifiedID &p_unifiedID){
			return (sceneObjectsRef->getByRef(p_unifiedID.sdkID).sceneObject);
		}
		inline static smPipe* getPipe(smUnifiedID &p_unifiedID){
			return (pipesRef->getByRef(p_unifiedID.sdkID).pipe);
		}
		inline static smPipe* getPipeByName(QString p_name){
			return (pipesRef->getByRef(p_name).pipe);
		}
		static void registerPipe(smPipe*p_pipe){
			smPipeHolder ph;
			ph.pipe=p_pipe;
			p_pipe->uniqueId.sdkID=pipesRef->checkAndAdd(ph);
		} 
		inline static smPipe *createPipe(QString p_pipeName,smInt p_elementSize,smInt p_size){
			smPipe *pipe;
			pipe=new smPipe(p_pipeName,p_elementSize,p_size);
			registerPipe(pipe);
			return pipe;
		}
		
		


		//static smInt registerSceneObject(smSceneObject *p_mesh);
		
		void handleEvent(smEvent *p_event);
	    
        
};

#endif