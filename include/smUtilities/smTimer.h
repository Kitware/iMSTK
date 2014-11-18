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
    \brief	    This Class had timing function. It is operating system dependent. 

*****************************************************
*/

#ifndef SMTIMER_H
#define SMTIMER_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"

#define SMTIMER_FRAME_MILLISEC2SECONDS(X) (   (smLongDouble)   (smLongDouble)X/1000.0)
#define SMTIMER_FRAME_MICROSEC2SECONDS(X)  (  (smLongDouble)   (smLongDouble)X /1000000.0)


enum smTimerType{
	SOFMIS_TIMER_INMILLISECONDS,
	SOFMIS_TIMER_INMICROSECONDS
};
#ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
	#include <windows.h> 
#endif

class smTimer:public smCoreClass{

public:
		smTimer(){
		  #ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
			 QueryPerformanceFrequency(&m_liPerfFreq);
		  #endif
		 start();
		}
	inline 	void start(){
			 #ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
				QueryPerformanceCounter(&m_liPerfStart);
			 #endif
		
		} 

	inline smLongDouble now(smTimerType p_type ) 	// Returns # of microseconds since Start was called
	{
		#ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
		smLongInt perSecond;
			if(p_type==SOFMIS_TIMER_INMILLISECONDS)
				 perSecond=1000;   //timer for milliseconds
			else
				 perSecond=1000000;//timer for microseconds


		
		QueryPerformanceCounter(&m_liPerfNow);
		return(((smLongDouble)(m_liPerfNow.QuadPart - m_liPerfStart.QuadPart)*perSecond)/( (smLongDouble)m_liPerfFreq.QuadPart));
		#endif
	}

private:



#ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
	LARGE_INTEGER m_liPerfFreq;		// Counts per second
	LARGE_INTEGER m_liPerfStart;	// Starting count
	LARGE_INTEGER m_liPerfNow;	// Starting count
#endif

};



#endif