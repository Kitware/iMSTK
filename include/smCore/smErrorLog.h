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

#ifndef SMERRORLOG_H
#define SMERRORLOG_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smUtilities/smTimer.h"

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>

///This is class is for error storing of the whole SimMedTK system.
///All errors should be reported to the instance of this class.
///the critcal part are implemented considering multiple inputs from different threads. Please
///read the function explanation for further details.
class smErrorLog: smCoreClass
{

private:
    ///total number of errors
    smInt errorCount;

    ///last error index
    smInt lastError;

    ///erros are stored in buffer
    smChar errors[SIMMEDTK_MAX_ERRORLOG][SIMMEDTK_MAX_ERRORLOG_TEXT];

    ///time stamps for the error registered in the buffer
    smInt timeStamp[SIMMEDTK_MAX_ERRORLOG];

    ///mutex to sync accesses
    std::mutex logLock;

    ///get the timing
    smTimer time;

public:
    smBool isOutputtoConsoleEnabled;
    smErrorLog();

    ///add the error in the repository.It is thread safe. It can be called by multiple threads.
    smBool addError(smCoreClass *p_param, const smChar *p_text);
    smBool addError(smCoreClass *p_param, const std::string p_text);
    smBool addError(const smChar *p_text);
    smBool addError(const std::string p_text);

    ///Clean up all the errors in the repository.It is thread safe.
    void cleanAllErrors();

    ///Print the last error.It is not thread safe.
    void printLastErr();

    ///Print  the last error in Thread Safe manner.
    void printLastErrSafe();

};

#endif
