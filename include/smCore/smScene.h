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
\brief	    This Class is scene. All pshyics entities should reside in the scene. The simulator will execute  all the physics in the scene

*****************************************************
*/


#ifndef SMSCENE_H
#define SMSCENE_H
#include <QVector>
#include <QMutex>
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smErrorLog.h"
#include "smUtilities/smDataStructs.h"
#include "smCore/smDoubleBuffer.h"

class smPipe;
//template<typename T> class smIndiceArray;

class smSDK;
//class smScene;

struct smSceneLocal{
public:
	
	smInt id;
	smSceneLocal(){
		sceneUpdatedTimeStamp=0;
		
	
	}
	inline smBool operator ==(smSceneLocal &p_param){
		return id==p_param.id;
	
	}
	vector<smSceneObject*> sceneObjects;
	smUInt sceneUpdatedTimeStamp;
};




///Physics class should have all parameters such as material properties, mesh etc.. for 
///note that when you remove the Physics do not delete it.Since propagation of the physics over the 
class smScene:public smCoreClass{
private:
	//smUnifiedID sceneId;


	smInt totalObjects;
																    
	smErrorLog *log;
	QMutex sceneList;
	smUInt referenceCounter;
	smUInt sceneUpdatedTimeStamp;
	vector<smSceneObject*> addQueue;
	smIndiceArray<smSceneLocal*> sceneLocal;
	QHash<smInt,smInt> sceneLocalIndex;
	vector<smSceneObject*> sceneObjects;
	

	void inline copySceneToLocal(smSceneLocal *p_local){
		p_local->sceneObjects.clear();
		
		for(smInt i=0;i<sceneObjects.size();i++)
			p_local->sceneObjects.push_back(sceneObjects[i]);
		p_local->sceneUpdatedTimeStamp=sceneUpdatedTimeStamp;
	
	}
	
public: 
	struct smSceneIterator{
		protected:
			//smInt startIndex;
			smInt endIndex;
			smInt currentIndex;
			smSceneLocal *sceneLocal;
		public:
		inline smSceneIterator(){
			currentIndex=endIndex=0;
			sceneLocal=NULL;

		}
		inline void setScene(smScene *p_scene,smCoreClass *p_core){
			
			sceneLocal=p_scene->sceneLocal.getByRef(p_scene->sceneLocalIndex[p_core->uniqueId.ID]);
			if(p_scene->sceneUpdatedTimeStamp>sceneLocal->sceneUpdatedTimeStamp){
			 p_scene->sceneList.lock();
					p_scene->copySceneToLocal(sceneLocal);
			 p_scene->sceneList.unlock();

			}
			
			 endIndex=sceneLocal->sceneObjects.size();
			 currentIndex=0;


		}
		inline smInt start(){
	  		return 0;
		}
		inline smInt end(){
			return endIndex;
		}
		
		inline void operator++(){
		  currentIndex++;
		}
		inline void operator--(){
		   currentIndex--;
		} 
		inline smSceneObject* operator[](smInt p_index){
			return sceneLocal->sceneObjects[p_index];
		}
		inline smSceneObject* operator*(){
			return sceneLocal->sceneObjects[currentIndex];
		}




	};
	//vector<smSceneObject*> sceneObjects;
	
	



	smScene(smErrorLog *p_log=NULL);

	void  registerForScene(smCoreClass *p_sofmisObject){
		smSceneLocal *local=new smSceneLocal();
		local->id=p_sofmisObject->uniqueId.ID;
		sceneList.lock();
			copySceneToLocal(local);
			sceneLocalIndex[p_sofmisObject->uniqueId.ID]=sceneLocal.checkAndAdd(local);
		sceneList.unlock();
	

	}
	
	


	///add physics in the scene
	void  addSceneObject(smSceneObject *p_sceneObject);
	///remove the phyics in the scene.
	///The removal of the phsyics in the scene needs some sync all over the modules
	///so not implemented yet. Be aware that when you remove the phyics do no free the smPhysics class
	void removeSceneObject(smSceneObject *p_sceneObject);
	///the same as 
	void removeSceneObject(smInt p_objectId);
	///in order to get the phsyics in the scene call this function.
	///it is thread safe. but it shouldn't be called frequently.
	///it should be called in the initialization of the viewer, simulation or any other module.
	///and the the list should be stored internally.
	///The scene list removal will be taken care of later since the list should be update.

	vector<smSceneObject*> getSceneObject();


	smInt getSceneId();

	inline smInt getTotalObjects();


	///Same functionality as addSceneObject
	smScene& operator +=(smSceneObject *p_sceneObject);


	void addRef();
	void removeRef();
	void copySceneObjects(smScene*p_scene);
	smScene &operator =(smScene &p_scene) ;

	friend class smSDK;
	//friend class smViewer;
	friend struct smSceneIterator;



   smInt test;
};


#endif
