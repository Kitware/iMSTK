/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
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
/// \brief maximum entities in the framework
#define SOFMIS_SDK_MAXMESHES 100
#define SOFMIS_SDK_MAXMODULES 100
#define SOFMIS_SDK_MAXOBJECTSIMULATORS 100
#define SOFMIS_SDK_MAXSCENES 100
#define SOFMIS_SDK_MAXSCENEOBJTECTS 100

class smMotionTransformer;
class smPipe;
template<typename T> class smIndiceArray;
class smScene;
/// \brief module registration
enum smSDKReturnType{
	SOFMIS_SDK_MODULEREGISTERED,
	SOFMIS_SDK_MODULEREGISTEREDALREADY
};

struct smBaseHolder{
};
/// \brief mesh holder
struct smMeshHolder:public smBaseHolder{

	smMeshHolder(){
		mesh=NULL;
	}

	smBaseMesh* mesh;

	inline smBool operator ==(smMeshHolder &p_param){
		return mesh==p_param.mesh;
	}
};
/// \brief module holder
struct smModuleHolder:public smBaseHolder{

	smModuleHolder(){
		module=NULL;
	}

	smModule* module;

	inline smBool operator ==(smModuleHolder &p_param){
		return module==p_param.module;
	}
};
/// \brief simulator holder
struct smObjectSimulatorHolder:public smBaseHolder{

	smObjectSimulatorHolder(){
		objectSim=NULL;
	}

	smObjectSimulator* objectSim;

	inline smBool operator ==(smObjectSimulatorHolder &p_param){
		return objectSim==p_param.objectSim;
	}
};
/// \brief scene holders
struct smSceneHolder:public smBaseHolder{
	smSceneHolder(){
		scene=NULL;
	}

	smScene* scene;
	inline smBool operator ==(smSceneHolder &p_param){
		return scene==p_param.scene;
	}

};
/// \brief scene object holder
struct smSceneObjectHolder:public smBaseHolder{

	smSceneObjectHolder(){
		sceneObject=NULL;
	}

	smSceneObject* sceneObject;
	inline smBool operator ==(smSceneObjectHolder &p_param){
		return sceneObject==p_param.sceneObject;
	}

};
/// \brief pipe holder
struct smPipeHolder:public smBaseHolder{

	smPipe *pipe;

	smPipeHolder(){
		pipe=NULL;
	}

	inline smBool operator ==(smPipeHolder &p_param){
		return pipe==p_param.pipe;
	}

	inline friend smBool operator==(smPipeHolder &p_pipe,QString &p_name){
		return (*(p_pipe.pipe)==p_name);
	}

};
/// \brief SDK class. it is a singlenton class for each machine runs the framework
class smSDK:public smCoreClass,public smEventHandler{

protected:
	///this id is incremented automatically when the scene is created.
	smInt sceneIdCounter;
	/// \brief pointer to the Qapplication
	QApplication *application;
	/// \brief pointers to the viewer, simulator
	smViewer *viewer;
	smSimulator *simulator;
	/// \brief scene list
	vector<smScene*>sceneList;
	/// \brief error log
	static smErrorLog *errorLog;
	/// \brief dispatcher
	smDispatcher *dispathcer;
	/// \brief event dispather
	smEventDispatcher *eventDispatcher;
	smInt argc;
	smChar argv;
	smBool isModulesStarted;
	/// \brief update scene list. not implemented
	void updateSceneListAll();
	/// \brief init registered modules
	void initRegisteredModules();
	/// \brief run the registered modules
	void runRegisteredModules();
	/// \brief singlenton sdk.
	static smSDK sdk;
	///holds the references to the entities in the framework
	static smIndiceArray<smMeshHolder> *meshesRef;
	static smIndiceArray<smModuleHolder> *modulesRef;
	static smIndiceArray<smObjectSimulatorHolder> *objectSimulatorsRef;
	static smIndiceArray<smObjectSimulatorHolder> *collisionDetectorsRef;
	static smIndiceArray<smSceneHolder> *scenesRef;
	static smIndiceArray<smSceneObjectHolder> *sceneObjectsRef;
	static smIndiceArray<smMotionTransformer *> *motionTransRef;
	static smIndiceArray<smPipeHolder>* pipesRef;
	/// \brief destructor
	~smSDK();
	/// \brief constructor
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
	///shutdowns all the modules
	void shutDown();

	///for now both functions below are the same. But it maybe subject to change.
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
	smScene *createScene();

	///SDK returns logger for the system
	static smErrorLog *getErrorLog(){
		return errorLog;
	};


	///SDK returns the event dispather
	inline smEventDispatcher *getEventDispatcher(){
		return eventDispatcher;
	}

	///terminates every module. Do it later on with smMessager
	void terminateAll(){

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
	/// \brief run the SDK
	void run();
	/// \brief add reference to a core class
	static void addRef(smCoreClass* p_coreClass);
	/// \brief removes reference on core class
	static void removeRef(smCoreClass* p_coreClass);

	/// \brief register functions
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


	/// \brief getter functions
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
	/// \brief register pipe
	static void registerPipe(smPipe*p_pipe){
		smPipeHolder ph;
		ph.pipe=p_pipe;
		p_pipe->uniqueId.sdkID=pipesRef->checkAndAdd(ph);
	}
	/// \brief create a pipe
	inline static smPipe *createPipe(QString p_pipeName,smInt p_elementSize,smInt p_size){
		smPipe *pipe;
		pipe=new smPipe(p_pipeName,p_elementSize,p_size);
		registerPipe(pipe);
		return pipe;
	}
	/// \brief handle an event
	void handleEvent(smEvent *p_event);

};

#endif
