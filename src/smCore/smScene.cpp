#include "smCore/smScene.h"
#include "smCore/smSDK.h"

smScene::smScene(smErrorLog *p_log):sceneLocal(SIMMEDTK_MAX_MODULES){

	type=SIMMEDTK_SMSCENE;
	this->log=p_log;
	totalObjects=0;
	test=0;
	sceneUpdatedTimeStamp=0;
}

smInt smScene::getSceneId(){

	return uniqueId.ID;
}

smInt smScene::getTotalObjects(){

	return totalObjects;
}

vector<smSceneObject*> smScene::getSceneObject(){
	
	sceneList.lock();
	return sceneObjects;
	sceneList.unlock();
}

smScene& smScene::operator +=(smSceneObject *p_sceneObject){

	addSceneObject(p_sceneObject);
	return *this;
}


void smScene::addSceneObject(smSceneObject *p_sceneObject){

	if(p_sceneObject!=NULL){
		sceneList.lock();
		sceneObjects.push_back(p_sceneObject);
		totalObjects=sceneObjects.size();
		sceneUpdatedTimeStamp++;
		sceneList.unlock();
	}
}

/// \brief removes the scene object based on scene object id
void smScene::removeSceneObject(smSceneObject *p_sceneObject){

	if(p_sceneObject!=NULL){
		sceneList.lock();
		for(smInt i=0;i<sceneObjects.size();i++){
			if(sceneObjects[i]==p_sceneObject){
				sceneObjects.erase(sceneObjects.begin()+i);
				totalObjects=sceneObjects.size();
			}
		}
		sceneUpdatedTimeStamp++;
		sceneList.unlock();
	}
}

/// \brief removes the object from the scene based on its object id
void smScene::removeSceneObject(smInt p_sceneObjectId){
	sceneList.lock();
	if(p_sceneObjectId>=0&&p_sceneObjectId<sceneObjects.size())
	{
		for(smInt i=0;i<sceneObjects.size();i++)
		{
			if(sceneObjects[i]->uniqueId==p_sceneObjectId){         
				sceneObjects.erase(sceneObjects.begin()+i);
				totalObjects=sceneObjects.size();
			}
		}

	}
	sceneUpdatedTimeStamp++;
	sceneList.unlock();

}

/// \brief
void smScene::addRef(){

	sceneList.lock();
		this->referenceCounter++;
	sceneList.unlock();
}

void smScene::removeRef(){

	sceneList.lock();
		this->referenceCounter--;
	sceneList.unlock();
}

void smScene::copySceneObjects(smScene*p_scene){

	p_scene->sceneObjects.clear();
	for(smInt i=0;i<this->sceneObjects.size();i++){
		p_scene->sceneObjects.push_back(sceneObjects[i]);	
	}
}

smScene &smScene::operator =(smScene &p_scene) {

	copySceneObjects(&p_scene);
	return *this;
}
