/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMERRORLOG_H
#define SMERRORLOG_H
#include <iostream>
#include <fstream>
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include <qmutex.h>
#include <QTime>
#include <string>

#include <iostream>
using namespace std;

///This is class is for error storing of the whole SOFMIS system.
///All errors should be reported to the instance of this class.
///the critcal part are implemented considering multiple inputs from different threads. Please
///read the function explanation for further details. 
class smErrorLog:smCoreClass{

private:
	///total number of errors
	smInt errorCount;

	///last error index
	smInt lastError;

	///erros are stored in buffer
	smChar errors[SOFMIS_MAX_ERRORLOG][SOFMIS_MAX_ERRORLOG_TEXT];

	///time stamps for the error registered in the buffer
	smInt timeStamp[SOFMIS_MAX_ERRORLOG];

	///mutex to sync accesses
	QMutex mutex;

	///get the timing
	QTime time;

public:
	smBool isOutputtoConsoleEnabled;
	smErrorLog();

	///add the error in the repository.It is thread safe. It can be called by multiple threads.
	smBool addError(smCoreClass *p_param, const smChar *p_text);
	smBool addError(smCoreClass *p_param, const string p_text);
	smBool addError(const smChar *p_text);
	smBool addError(const string p_text);

	///Clean up all the errors in the repository.It is thread safe.
	void cleanAllErrors();

	///Print the last error.It is not thread safe.
	void printLastErr();

	///Print  the last error in Thread Safe manner.
	void printLastErrSafe();

};

#endif
