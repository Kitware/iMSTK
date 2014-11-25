#include "smCore/smErrorLog.h"

smErrorLog::smErrorLog(){

	time.start();
	errorCount=-1;
	isOutputtoConsoleEnabled=true;
}

///add the error in the repository.It is thread safe. It can be called by multiple threads.
smBool smErrorLog::addError(smCoreClass *p_param, const smChar *p_text){

	smInt textLength;
	
	if(p_text==NULL)
		return false;
	
	textLength=strlen(p_text);
	
	if(textLength<SOFMIS_MAX_ERRORLOG_TEXT&textLength>0){
		mutex.lock();
		if(isOutputtoConsoleEnabled)
			cout<<p_text<<endl;

		strcpy(errors[errorCount],p_text);
		timeStamp[errorCount]=time.elapsed();
		lastError=errorCount;

		errorCount++;
		if(errorCount>=SOFMIS_MAX_ERRORLOG){
			lastError=SOFMIS_MAX_ERRORLOG-1;
			errorCount=0;
		}
		mutex.unlock();
		return true;
	}
	else 
		return false;
}

smBool smErrorLog::addError(smCoreClass *p_param, const string p_text){
	return addError(p_param, p_text.c_str());
}

smBool smErrorLog::addError(const smChar *p_text){
	
	smInt textLength;
	
	if(p_text==NULL)
		return false;

	textLength=strlen(p_text);
	
	if(textLength<SOFMIS_MAX_ERRORLOG_TEXT&textLength>0){
		mutex.lock();
		errorCount++;
		strcpy(errors[errorCount],p_text);
		timeStamp[errorCount]=time.elapsed();
		lastError=errorCount;

		if(errorCount>=SOFMIS_MAX_ERRORLOG){
			lastError=SOFMIS_MAX_ERRORLOG-1;
			errorCount=0;
		}
		mutex.unlock();
		return true;
	}
	else 
		return false;
}

smBool smErrorLog::addError(const string p_text){
	return addError(p_text.c_str());
}

///Clean up all the errors in the repository.It is thread safe.
void smErrorLog::cleanAllErrors(){

	mutex.lock();
	for(smInt i=0;i<SOFMIS_MAX_ERRORLOG;i++){
		memset(errors[i],'\0',SOFMIS_MAX_ERRORLOG_TEXT);
	}
	mutex.unlock();
}

///Print the last error.It is not thread safe.
void smErrorLog::printLastErr(){

	if(errorCount!=-1)
		cout<<"Last Error:"<<errors[errorCount]<<" Time:"<<timeStamp[errorCount-1]<<" ms"<<endl;
}

///Print  the last error in Thread Safe manner.
void smErrorLog::printLastErrSafe(){
	
	mutex.lock();
	cout<<"Last Error:"<<errors[errorCount-1]<<" Time:"<<timeStamp[errorCount-1]<<" ms"<<endl;
	mutex.unlock();
}
