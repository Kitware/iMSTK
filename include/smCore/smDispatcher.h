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
    \brief	    This Module is Dispathcer. After each loop all modules should call dispathcer function so that if
                particular event or any update happens, this will take care of this.
    \brief      Module is in test now

*****************************************************
*/

#ifndef SMDISPATCHER_H
#define SMDISPATCHER_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"

///The dispacther class is responsible of taking care of the 
///tasks that need to proppgated over the whole modules.For instance, adding a new 
///phyiscs at run-time
class smDispatcher:public smCoreClass{
    private:
       ///handle viewer events
       /// \param p_caller this is the caller class
        /// \param p_callerState it indicates the state of the called which is predefined at the smConfig.h
       void  handleViewer(smCoreClass *p_caller,smCallerState p_callerState);
       ///handle simulator events
       void  handleSimulator(smCoreClass *p_caller,smCallerState p_callerState);
       ///handle collision detection events
       void  handleCollisionDetection(smCoreClass *p_caller,smCallerState p_callerState);
       ///handle all events
       void  handleAll();
       
    public:
        smDispatcher(){
            type=SOFMIS_SMDISPATHCER;
        
        
        }
    public:
	///handle all event and call corresponding events
    smDispathcerResult handle(smCoreClass *p_caller,smCallerState p_callerState);
 
};

#endif