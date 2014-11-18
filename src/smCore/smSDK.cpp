#include <QMutex>

#include "smCore/smSDK.h"
#include "smMesh/smMesh.h"

///SDK is singlenton class
smSDK smSDK::sdk;



smErrorLog * smSDK::errorLog;


///object reference counter mutex
QMutex  objectRefMutex;


///this mutex is for system global registration
QMutex  globalRegisterMutex;
QHash<smInt,smSceneObject*> sceneObjectList;
//QHash<smInt,smMesh*> meshList;

smIndiceArray<smMeshHolder>  *smSDK::meshesRef;
smIndiceArray<smModuleHolder> *smSDK::modulesRef;
smIndiceArray<smObjectSimulatorHolder>  *smSDK::objectSimulatorsRef;
smIndiceArray<smObjectSimulatorHolder>*smSDK::collisionDetectorsRef;
smIndiceArray<smSceneHolder>*smSDK::scenesRef;
smIndiceArray<smSceneObjectHolder>*smSDK::sceneObjectsRef;
smIndiceArray<smMotionTransformer *> *smSDK::motionTransRef;
smIndiceArray<smPipeHolder>	*smSDK::pipesRef;



///creates the scene of the simulator
smScene  *smSDK::createScene(){
	smScene*scene;
	scene=new smScene(errorLog);

	
	//do registration 
	//sceneMutex.lock();
		//sceneList.push_back(scene);
		//scene->sceneId=sceneIdCounter;
		//scene->setName("Scene"+scene->sceneId);

		//sceneIdCounter++;
		registerScene(scene);
		scene->setName(QString("Scene")+QString().setNum(scene->uniqueId.ID));
	//sceneMutex.unlock();

	return scene;
 }

smSDK::~smSDK(){




}

///creates the viewer for the simulator
smViewer *smSDK::createViewer(){
   
	
	if(this->viewer==NULL){

		viewer=new smViewer(errorLog);
		
		viewer->dispathcer=dispathcer;
		//sceneMutex.lock();
		/*viewer->sceneList=this->sceneList;
		for(smInt index=0;index<this->sceneList.size();index++)
		{
		   viewer->sceneObjectList.push_back(sceneList.at(index)->sceneObjects);
			
		}*/

		for(smInt j=0;j<(*scenesRef).size();j++){
			viewer->sceneList.push_back((*scenesRef)[j].scene);
			
			//viewer->sceneObjectList.push_back((*scenesRef)[j].scene->sceneObjects);
		}

		//sceneMutex.unlock();
		registerModule(viewer);
		

		
	}
	
	return viewer;

}
smSimulator* smSDK::createSimulator(){
   
	
	if(this->simulator==NULL){

		simulator=new smSimulator(errorLog);
		simulator->dispathcer=dispathcer;
		//sceneMutex.lock();
		
		//simulator->sceneList=this->sceneList;
		/*for(smInt index=0;index<this->sceneList.size();index++)
		{
		   simulator->sceneObjectList.push_back(sceneList.at(index)->sceneObjects);
			
		}*/
		//sceneMutex.unlock();
		for(smInt j=0;j<(*scenesRef).size();j++){
			simulator->sceneList.push_back((*scenesRef)[j].scene);
			
			//simulator->sceneObjectList.push_back((*scenesRef)[j].scene->sceneObjects);
		}

		
		registerModule(simulator);
	}
	return simulator;
}


void smSDK::updateSceneListAll(){

	QHash<smInt, smModule*>::iterator moduleIterator;
	//sceneMutex.lock();
	//if(simulator!=NULL){
	//	
	//	
	//	
	//	simulator->sceneList=this->sceneList;

 //       for(smInt index=0;index<this->sceneList.size();index++)
 //          simulator->sceneObjectList.push_back(sceneList.at(index)->sceneObjects);
 //           
 //       
	//}
	//if(viewer!=NULL){
	//	viewer->sceneList=this->sceneList;
 //       for(smInt index=0;index<this->sceneList.size();index++)
 //          viewer->sceneObjectList.push_back(sceneList.at(index)->sceneObjects);
 //     
	//}
	//for (moduleIterator = modules.begin(); moduleIterator != modules.end(); ++moduleIterator){
	//	// cout << moduleIterator.key() << ": " << moduleIterator.value() << endl;
	//	for(smInt index=0;index<this->sceneList.size();index++)
	//		(moduleIterator.value())->sceneObjectList.push_back(sceneList.at(index)->sceneObjects);
	//}
 //	sceneMutex.unlock();
}

///Initialize all modules registered to the Sofmis SDK
void smSDK::initRegisteredModules(){
	/*QHash<smInt, smModule*>::iterator moduleIterator;
	for (moduleIterator = modules.begin(); moduleIterator != modules.end(); ++moduleIterator){
		if(moduleIterator.value()->getType()!=SOFMIS_SMVIEWER)
			moduleIterator.value()->init();
		
	}*/

	for(smInt i=0;i<modulesRef->size();i++)
	 if((*modulesRef)[i].module->getType()!=SOFMIS_SMVIEWER)
		(*modulesRef)[i].module->init();


}

void smSDK::runRegisteredModules(){
	if(isModulesStarted)
		return;
	//moduleMutex.lock();
	/*QHash<smInt, smModule*>::iterator moduleIterator;
	for (moduleIterator = modules.begin(); moduleIterator != modules.end(); ++moduleIterator){
		moduleIterator.value()->exec();
		
	}*/

	for(smInt i=0;i<modulesRef->size();i++)
		(*modulesRef)[i].module->exec();


	isModulesStarted=true;
	//moduleMutex.unlock();


}


void smSDK::shutDown(){
	  for(smInt i=0;i<modulesRef->size();i++)
		  (*modulesRef)[i].module->terminateExecution=true;

}

///runs the simulator
void smSDK::run(){
	updateSceneListAll();
	initRegisteredModules();
	//previous code where explicit code written for all modules
	if(viewer==NULL)
		errorLog->addError(this,"smSDK:Viewer is not created");
	else
		viewer->exec();
	if(simulator!=NULL)
		simulator->exec();
	runRegisteredModules();
	


	
		 

	application->exec();
	terminateAll();
	
								
}

 void smSDK::addRef(smCoreClass* p_coreClass){
	objectRefMutex.lock();
		p_coreClass->referenceCounter++;
	objectRefMutex.unlock();
}
 void smSDK::removeRef(smCoreClass* p_coreClass){
	objectRefMutex.lock();
		p_coreClass->referenceCounter--;
	objectRefMutex.unlock();
}

 //void smSDK::registerMesh(smMesh*p_mesh){
	////globalRegisterMutex.lock();	 
	// //meshList.insert(p_mesh->meshId.ID,p_mesh);
	// //meshesRef[smObjectHolder<smMesh*>.]
	////globalRegisterMutex.unlock();
 //}

 //void smSDK::registerSceneObject(smSceneObject *p_sceneObject){
	// globalRegisterMutex.lock();
	//	sceneObjectList.insert(p_sceneObject->sceneObjectId.ID,p_sceneObject);
	// globalRegisterMutex.unlock();
 //
 //}


 void smSDK::handleEvent(smEvent *p_event){
			smKeyboardEventData *keyBoardData;
			
			switch(p_event->eventType.eventTypeCode){

				case SOFMIS_EVENTTYPE_KEYBOARD:
					keyBoardData=(smKeyboardEventData*)p_event->data;
					if(keyBoardData->keyBoardKey==Qt::Key_Escape)
						terminateAll();
					break;
			}
	
}
 