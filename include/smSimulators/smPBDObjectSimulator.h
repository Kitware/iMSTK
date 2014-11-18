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
\brief	    This class is the simulator object. Each simulator should derive this.



*****************************************************
*/

#ifndef SMPBDOBJECTSIMULATOR_H
#define SMPBDOBJECTSIMULATOR_H
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"


///example simulator..the dummy simulator works on static scene objects for now..
class smPBDObjectSimulator:public smObjectSimulator,public smEventHandler{

private:


public:
	smPBDObjectSimulator(smErrorLog *p_errorLog):smObjectSimulator(p_errorLog){
		this->scheduleGroup.maxTargetFPS=100;
		this->scheduleGroup.minTargetFPS=30;



	}


protected:
	
	
	inline void initObject(smPBDSurfaceSceneObject *p_object){
		if(p_object->flags.isSimulatorInit)
			return;
		//p_object->InitSurfaceObject();
		p_object->memBlock->allocate< smVec3<smFloat> >(QString("pos"),p_object->nbrMass);
		p_object->memBlock->originaltoLocalBlock(QString("pos"),p_object->mesh->vertices,p_object->mesh->nbrVertices);
		p_object->flags.isSimulatorInit=true;
	
	
	}
	virtual void initCustom(){
		smClassType type;
		smSceneObject *object;
		smPBDSurfaceSceneObject *pbdSurfaceSceneObject;
		smVec3<smFloat> *newVertices;
		//do nothing for now
		for(smInt i=0;i<objectsSimulated.size();i++){
			object=objectsSimulated[i];
			type=object->getType();
			switch(type){ 
				case SOFMIS_SMPBDSURFACESCENEOBJECT:
					pbdSurfaceSceneObject=(smPBDSurfaceSceneObject*)object;
					initObject(pbdSurfaceSceneObject);

					break;
			}


		}

	}


	//test
	virtual void run(){
		smSceneObject *sceneObj;
		smPBDSurfaceSceneObject *pbdSurfaceSceneObject;
		smVec3<smFloat> *X;
		smMesh *mesh;
		smInt i;
		smFloat dist, lamda;
		smVec3<smFloat> dirVec, dP;
		smInt count=0;
		smInt a, b;

		beginSim();
		for(smInt j=0;j<this->objectsSimulated.size();j++){
			sceneObj=this->objectsSimulated[j];

			//ensure that dummy simulator will work on static scene objects only.
			if(sceneObj->getType()==SOFMIS_SMPBDSURFACESCENEOBJECT){
				pbdSurfaceSceneObject=(smPBDSurfaceSceneObject*)sceneObj;
				if(!pbdSurfaceSceneObject->flags.isSimulatorInit)
					initObject(pbdSurfaceSceneObject);

				mesh=pbdSurfaceSceneObject->mesh;

				pbdSurfaceSceneObject->memBlock->getBlock(QString("pos"),(void**)&X);

				for(i=0;i<pbdSurfaceSceneObject->nbrMass;i++) pbdSurfaceSceneObject->exF[i].setValue(0.0,0.0,0.0);
				for(i=0;i<pbdSurfaceSceneObject->nbrMass;i++) pbdSurfaceSceneObject->exF[i].y -= 1.0;
				//exF[nbrMass/2].setValue(0.0,0.0,0.0);

				for(i=0;i<pbdSurfaceSceneObject->nbrMass;i++){
					pbdSurfaceSceneObject->V[i] = pbdSurfaceSceneObject->V[i] + (pbdSurfaceSceneObject->exF[i] - pbdSurfaceSceneObject->V[i] * pbdSurfaceSceneObject->Damp) * pbdSurfaceSceneObject->dT;
					if(!pbdSurfaceSceneObject->fixedMass[i]) 
						pbdSurfaceSceneObject->P[i] = X[i] + pbdSurfaceSceneObject->V[i] * pbdSurfaceSceneObject->dT;
				}



				count = 0;
				while(count<30){

					for(i=0;i<pbdSurfaceSceneObject->nbrSpr;i++){
						//a = pbdSurfaceSceneObject->sprIdx[i][0];
						//b = pbdSurfaceSceneObject->sprIdx[i][1];

						a = pbdSurfaceSceneObject->mesh->edges[i].vert[0];
						b = pbdSurfaceSceneObject->mesh->edges[i].vert[1];

						dirVec = pbdSurfaceSceneObject->P[a] - pbdSurfaceSceneObject->P[b];
						dist = dirVec.module();
						dirVec = dirVec/dist;

						lamda = 0.5 * (dist - pbdSurfaceSceneObject->L0[i]);
						dP = dirVec * (lamda *pbdSurfaceSceneObject-> paraK);

						//dP.setValue(0.0,0.0,0.0);

						if(pbdSurfaceSceneObject->fixedMass[a]&&(!pbdSurfaceSceneObject->fixedMass[b])){
							pbdSurfaceSceneObject->P[b] = pbdSurfaceSceneObject->P[b] + dP * 2.0;
						}
						else if((!pbdSurfaceSceneObject->fixedMass[a])&&pbdSurfaceSceneObject->fixedMass[b]){
							pbdSurfaceSceneObject->P[a] =pbdSurfaceSceneObject-> P[a] - dP * 2.0;
						}
						else if((!pbdSurfaceSceneObject->fixedMass[a])&&(!pbdSurfaceSceneObject->fixedMass[b])){
							pbdSurfaceSceneObject->P[a] =pbdSurfaceSceneObject-> P[a] - dP;
							pbdSurfaceSceneObject->P[b] = pbdSurfaceSceneObject->P[b] + dP;
						}
						else{
						}
					}
					count++;
				}

				for(i=0;i<pbdSurfaceSceneObject->nbrMass;i++){
					pbdSurfaceSceneObject->V[i] = (pbdSurfaceSceneObject->P[i] - X[i])/pbdSurfaceSceneObject->dT;
					if(!pbdSurfaceSceneObject->fixedMass[i]) X[i] = pbdSurfaceSceneObject->P[i];
				}





			}



		}
		//wait
		//do the job
		//write the result
		endSim();





	}

	

	///synchronize the buffers in the object..do not call by yourself.
	void syncBuffers(){
		smSceneObject *sceneObj;
		smPBDSurfaceSceneObject *pbdSurfaceSceneObject;
		smVec3<smFloat> *vertices;
		//smMesh *mesh;

		for(smInt i=0;i<this->objectsSimulated.size();i++){
			sceneObj=this->objectsSimulated[i];
			//ensure that dummy simulator will work on static scene objects only.
			if(sceneObj->getType()==SOFMIS_SMPBDSURFACESCENEOBJECT){
				pbdSurfaceSceneObject=(smPBDSurfaceSceneObject*)sceneObj;
				// mesh=staticSceneObject->mesh;
				pbdSurfaceSceneObject->memBlock->localtoOriginalBlock(QString("pos"),pbdSurfaceSceneObject->mesh->vertices,pbdSurfaceSceneObject->mesh->nbrVertices);
				pbdSurfaceSceneObject->mesh->updateTriangleNormals();
				pbdSurfaceSceneObject->mesh->updateVertexNormals();
				pbdSurfaceSceneObject->mesh->updateTriangleAABB();


			}



		}


	}

	void handleEvent(smEvent *p_event){
		smKeyboardEventData *keyBoardData;

		switch(p_event->eventType.eventTypeCode){

		case SOFMIS_EVENTTYPE_KEYBOARD:
			keyBoardData=(smKeyboardEventData*)p_event->data;
			if(keyBoardData->keyBoardKey==Qt::Key_F1)
				printf("F1 Keyboard is pressed %c\n",keyBoardData->keyBoardKey);
			break;



		}

	}


	void draw(smDrawParam p_params);

};






#endif