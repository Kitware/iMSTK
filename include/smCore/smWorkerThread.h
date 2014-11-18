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
    \brief	    this class is generic vertasitel worker thread  
*****************************************************
*/

#ifndef SMWORKERTHREAD_H
#define SMWORKERTHREAD_H

#include "smCore/smCoreClass.h"
#include "smCore/smSynchronization.h"
#include <Qthread>

enum smProcessNumbering{
  SOFMIS_PROCNUMSCHEME_X__,
  SOFMIS_PROCNUMSCHEME_XY_,
  SOFMIS_PROCNUMSCHEME_XYZ,
};

struct smProcessID{
public:
  smUShort x;
  smUShort y;
  smUShort z;
  smUShort totalProcX;
  smUShort totalProcY;
  smUShort totalProcZ;
  void *data;
  smInt sizeOfData;
  smProcessNumbering numbScheme;

  smProcessID(){
	x=y=z=totalProcX=totalProcY=totalProcZ=sizeOfData=0;
	data=NULL;
	numbScheme=SOFMIS_PROCNUMSCHEME_X__;
  }

  inline void operator=(smProcessID p_ID){
	  x=p_ID.x;
	  y=p_ID.y;
	  z=p_ID.z;
	  totalProcX=p_ID.totalProcX;
	  totalProcY=p_ID.totalProcY;
	  totalProcZ=p_ID.totalProcZ;

	  data=new smChar[p_ID.sizeOfData];
	  memcpy(data,p_ID.data,sizeOfData);
  }

};




class smProcess:public smCoreClass{

protected:
    
	smProcessID id;
	
	smBool termination;
public:
	smProcess(){
		id.x=0;
		id.y=0;
		id.z=0;
		id.totalProcX=0;
		id.totalProcY=0;
		id.totalProcZ=0;
		id.data=NULL;
		id.sizeOfData=0;
		id.numbScheme=SOFMIS_PROCNUMSCHEME_X__;
		termination=false;
	}
	smProcess(smProcessID p_id){
		id=p_id;
		termination=false;
		
	
	}
	
	
	void setId(smProcessID p_id){
		id=p_id;
		
	
	}
	
	virtual void kernel()=0; 
	void terminate(){
		termination=true;
	}

};

class smWorkerThread:public QThread,public smProcess{
protected:	
	smSynchronization *synch;
public:
	
	smWorkerThread(){
		
	
	}
	smWorkerThread(smProcessID p_ID):smProcess(p_ID)
	{
	
		termination=false;
	
	
	}
	smWorkerThread(smSynchronization &p_synch,smProcessID p_ID):smProcess(p_ID)
	{
		synch=&p_synch;
		termination=false;
		
		
	
	
	}
	void setSynchObject(smSynchronization &p_synch){
		synch=&p_synch;
	
	}
	
	virtual void run()=0;


};



#endif