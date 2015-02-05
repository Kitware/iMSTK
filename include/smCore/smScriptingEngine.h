// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------
 
#ifndef SMSCRIPTINGENGINE_H
#define SMSCRIPTINGENGINE_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include <QHash>

struct smClassVariableInfo
{
    void *variablePtr;
    QString className;
    QString variableName;
    SIMMEDTK_TYPEINFO type;
    smInt nbr;
};

enum smScriptReturnCodes
{
    SMSCRIPT_REG_OK,
    SMSCRIPT_REG_VARIABLENAMEEXISTS,
    SMSCRIPT_REG_VARIABLENAMEMISSING,
    SMSCRIPT_REG_CLASSNAMEEXISTS,
    SMSCRIPT_REG_CLASSNAMEMISSING,
};

class smScriptingEngine: public  smCoreClass
{

    QHash <QString, smClassVariableInfo*> registeredVariables;
    QHash <QString, smCoreClass*> registeredClasses;

public:

    smScriptReturnCodes registerVariable(smCoreClass *p_coreClass, void *p_variablePtr,
                                         SIMMEDTK_TYPEINFO p_typeInfo,
                                         QString p_variableName, smInt p_nbr)
    {

        smScriptReturnCodes ret = SMSCRIPT_REG_OK;
        QString nameID;
        smClassVariableInfo *variableInfo = new smClassVariableInfo();

        if (p_variableName.isEmpty())
        {
            return SMSCRIPT_REG_VARIABLENAMEMISSING;
        }

        if (p_coreClass->getName().isEmpty())
        {
            return SMSCRIPT_REG_CLASSNAMEMISSING;
        }

        variableInfo->type = p_typeInfo;
        variableInfo->variablePtr = p_variablePtr;
        variableInfo->className = p_coreClass->getName();
        variableInfo->variableName = p_variableName;
        nameID = p_coreClass->getName() + p_variableName;

        if (!registeredVariables.contains(nameID))
        {
            registeredVariables.insert(nameID, variableInfo);
        }
        else
        {
            ret = SMSCRIPT_REG_VARIABLENAMEEXISTS;
        }

        if (!registeredClasses.contains(p_coreClass->getName()))
        {
            registeredClasses.insert(p_coreClass->getName(), p_coreClass);
        }
        else
        {
            ret = SMSCRIPT_REG_CLASSNAMEEXISTS;
        }

        return ret;
    }

    void list();
};

#endif
