#include <iostream>
#include "smCore/smScriptingEngine.h"
using namespace std;

void smScriptingEngine::list()
{

    cout << "List of Registered Variables" << endl;
    foreach(smClassVariableInfo * info, registeredVariables)
    {
        cout << qPrintable(info->className) << "-->" << qPrintable(info->variableName) << endl;
    }

    cout << "List of Registered Classes" << endl;

    foreach(smCoreClass * classInfo, registeredClasses)
    cout << qPrintable(classInfo->getName()) << endl;
}
