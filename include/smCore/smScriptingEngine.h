#ifndef SMSCRIPTINGENGINE_H 
#define SMSCRIPTINGENGINE_H 
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include <QHash>

struct smClassVariableInfo{
	void *variablePtr;
	//smUInt variableSize;
	QString className; 
	QString variableName; 
	SOFMIS_TYPEINFO type;
	smInt nbr;

};

enum smScriptReturnCodes{
	SMSCRIPT_REG_OK,
	SMSCRIPT_REG_VARIABLENAMEEXISTS,
	SMSCRIPT_REG_VARIABLENAMEMISSING,
	SMSCRIPT_REG_CLASSNAMEEXISTS,
	SMSCRIPT_REG_CLASSNAMEMISSING,



};

class smScriptingEngine:public  smCoreClass{
	QHash <QString,smClassVariableInfo*> registeredVariables;
	QHash <QString,smCoreClass*> registeredClasses;


public:	

	smScriptReturnCodes registerVariable(smCoreClass *p_coreClass,void *p_variablePtr, 	SOFMIS_TYPEINFO p_typeInfo,	QString p_variableName,	smInt p_nbr)
	{
		smScriptReturnCodes ret=SMSCRIPT_REG_OK;
		QString nameID;
		smClassVariableInfo *variableInfo=new smClassVariableInfo();
		if(p_variableName.isEmpty())
			return SMSCRIPT_REG_VARIABLENAMEMISSING;
		if(p_coreClass->getName().isEmpty())
			return SMSCRIPT_REG_CLASSNAMEMISSING;
		variableInfo->type=p_typeInfo;
		variableInfo->variablePtr=p_variablePtr;
		variableInfo->className=p_coreClass->getName();
		variableInfo->variableName=p_variableName;
		nameID=p_coreClass->getName()+p_variableName;
		if(!registeredVariables.contains(nameID)){
			registeredVariables.insert(nameID,variableInfo);
		}
		else
			ret=SMSCRIPT_REG_VARIABLENAMEEXISTS;

		if(!registeredClasses.contains(p_coreClass->getName())){
			registeredClasses.insert(p_coreClass->getName(),p_coreClass);
		}
		else{
			ret=SMSCRIPT_REG_CLASSNAMEEXISTS;
		}

		return	 ret;
	}


	void list();


};




#endif