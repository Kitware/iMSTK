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
    \date       04/2009
    \bug	    None yet
    \brief	    This Module declares Core class of SOFMIS which should be derived by all main core modules except
                Vector class. This is due to performance issues;

*****************************************************
*/

#ifndef SMCORECLASS_H
#define SMCORECLASS_H
#include <QString>
#include <iostream>

#include "smCore/smConfig.h"
#include "smRendering/smConfigRendering.h"

class smSDK;
class smCoreClass;
class smObjectSimulator;
class smViewer;




struct smDrawParam{
  smViewer    *rendererObject;
  smCoreClass *caller;
  void *data;
};

struct smSimulationParam{
  smObjectSimulator *objectSimulator;
  void *caller;
  void *data;

};


///This class indicates the unified id of the objects
struct smUnifiedID{
private:

   static QAtomicInt IDcounter;
   
    smShort sdkID;
public:
  
  smShort ID;
  smShort machineID;
  smUnifiedID(){
	  sdkID=-1;
	  machineID=-1;

  }
  inline void generateUniqueID(){
   	  ID=IDcounter.fetchAndAddOrdered(1);
  }
  inline smInt getSDKID()const
  {
	return sdkID;
  }
  inline void operator =(const smUnifiedID &p_id){
	ID=p_id.ID;
	sdkID=p_id.sdkID;
	machineID=p_id.machineID;
  }
  inline bool operator==(smUnifiedID &p_id){
	return (ID==p_id.ID&&machineID==p_id.machineID);
  }
  inline bool operator==(smInt &p_ID){
	return (ID==p_ID);
  }
  inline bool operator!=(smInt &p_ID){
	return (ID!=p_ID);
  }
  friend smSDK;
};


class smCoreClass{
    protected:
		
        smClassType type;
		smInt referenceCounter;
		
		
		
    public:
		QString name;
		smUnifiedID uniqueId;
		smRenderDetail renderDetail;
		smClassDrawOrder drawOrder;
		smCoreClass():name(""){
			referenceCounter=0;
			drawOrder=SOFMIS_DRAW_BEFOREOBJECTS;
			uniqueId.generateUniqueID();
		
		}
	
        inline smClassType getType(){
            return type;
        }
		///This function is called by the renderer. The p_params stores renderer pointers
		virtual void initDraw(smDrawParam p_params){
		
		}
		virtual void draw(smDrawParam p_params){
		
		}

		virtual void initSimulate(smSimulationParam p_params){
		}
		virtual void simulate(smSimulationParam p_params){
		}

		virtual void print(){
			   
		}
		void setName(QString p_objectName){
			name=p_objectName;
			
		}
		QString getName(){
			return name;
		
		}
		/*///All classes should copy itself
		virtual void *clone(){
		
		}*/
		
		

    
   friend smSDK;
};

class smInterface{


};





#endif