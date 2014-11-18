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
\brief	    This Module has the data strutures and some utility functions for sofmis scheduler. 

*****************************************************
*/

#ifndef SMSCHEDULER_H
#define SMSCHEDULER_H

class smSimulator;
//class smObjectSimulatorObjectIter;

enum smSchedulingPriority{
  SOFMIS_SCHEDULING_HIGHRATE,
  SOFMIS_SCHEDULING_AUTOSCHEDULE,
  SOFMIS_SCHEDULING_BACKGROUND
};

struct smScheduleGroup{

	#ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
		HANDLE	threadHandle;
	#endif

//protected:
public:	
	smFloat groupWeight;
	smShort totalThreads;
	
	smFloat maxObservedFPS;
	smFloat minObservedFPS;
	smShort minObservedFPSThreadIndex;
	smShort maxObservedFPSThreadIndex;
	
public:
	smScheduleGroup(){
		totalThreads=1;
		minTargetFPS=maxTargetFPS=maxObservedFPS=minObservedFPS=0;

		minObservedFPSThreadIndex=maxObservedFPSThreadIndex=0;
		#ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
			threadHandle=GetCurrentThread();
		#endif 

	}
	smFloat maxTargetFPS;
	smFloat minTargetFPS;
	friend class smSimulator;
	//friend class smObjectSimulatorObjectIter;
};



#endif