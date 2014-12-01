/*****************************************************
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
    \brief	    This Module declares Utilities funciton,declarations nad parameters

*****************************************************/
#ifndef SMUTILS_H
#define SMUTILS_H


#define SM_CHECKGLERROR_DISPLAY(log,error)  smGLUtils::queryGLError(error);\
                                            log->addError(this,error);\
                                            log->printLastErr();\

#define SM_CHECKERROR(log,error) smGLUtils::queryGLError(error);\
                                 log->addError(this,error);\

#endif
