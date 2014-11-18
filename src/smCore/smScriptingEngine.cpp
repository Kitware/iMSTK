#include <iostream>
#include "smCore/smScriptingEngine.h"
using namespace std;


//template< class T>
//smScriptReturnCodes smScriptingEngine::registerVariable(smCoreClass *p_coreClass,T *p_variablePtr, QString p_variableName){
// smScriptReturnCodes ret=SMSCRIPT_REG_OK;
// QString nameID;
//
// smClassVariableInfo *variableInfo=new smClassVariableInfo();
// if(p_variableName.isEmpty())
//	 return SMSCRIPT_REG_VARIABLENAMEMISSING;
// if(p_coreClass->getName().isEmpty())
//	 return SMSCRIPT_REG_CLASSNAMEMISSING;
//
// variableInfo.variableSize=sizeof(T);
// variableInfo.variablePtr=p_variablePtr;
// variableInfo.className=p_coreClass->getName();
// variableInfo.variableName=p_variableName;
// nameID=p_coreClass->name+p_variableName;
// if(!registeredVariables.contains(nameID)){
//	 registeredVariables.insert(nameID,variableInfo);
//	
// }
// else
//	 ret=SMSCRIPT_REG_VARIABLENAMEEXISTS;
// 
// if(!registeredClasses.contains(p_coreClass->name)){
//	 registeredClasses.insert(p_coreClass->name,p_coreClass);
//	
// }
// else{
//	ret=SMSCRIPT_REG_CLASSNAMEEXISTS
// }
//
// return	 ret;
//
//  //typeid(p_coreClass).
//
//}
//

void smScriptingEngine::list(){

   cout<<"List of Registered Variables"<<endl;
   foreach (smClassVariableInfo *info, registeredVariables)
   { 
	   cout <<qPrintable(info->className) <<"-->"<< qPrintable(info->variableName) << endl;
   
   
   }
   cout<<"List of Registered Classes"<<endl;
   foreach (smCoreClass *classInfo, registeredClasses)
   { 
	   cout <<qPrintable(classInfo->getName())<< endl;
   
   
   }
   
   
	


}