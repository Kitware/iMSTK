/*****************************************************
                  SIMMEDTK LICENSE
****************************************************

*****************************************************/
#ifndef SMUTILS_H
#define SMUTILS_H

/// \brief query,  logs and diplays opengl error
#define SM_CHECKGLERROR_DISPLAY(log,error)  smGLUtils::queryGLError(error);\
                                            log->addError(this,error);\
                                            log->printLastErr();\
/// \brief query,  logs opengl error 
#define SM_CHECKERROR(log,error) smGLUtils::queryGLError(error);\
                                 log->addError(this,error);\
 
#endif
