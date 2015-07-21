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

#include "Core/ErrorLog.h"

#include <cstring>
#include <string>
#include <chrono>

ErrorLog::ErrorLog()
{

    time.start();
    consoleOutput = true;
}

bool ErrorLog::addError(const std::string& p_text)
{
    if (0 >= p_text.length())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(logLock); //Lock is released when leaves scope

    if (consoleOutput)
    {
        std::cout << p_text << "\n";
    }

    errors.push_back(p_text);
    timeStamps.push_back(time.elapsed() * 1000);

    if (100 <= errors.size())
    {
        errors.erase(errors.begin()); //Make room for new errors
    }

    return true;
}

///Clean up all the errors in the repository.It is thread safe.
void ErrorLog::cleanAllErrors()
{
    std::lock_guard<std::mutex> lock(logLock); //Lock is released when leaves scope
    errors.clear();
    timeStamps.clear();
}

///Print the last error.It is not thread safe.
void ErrorLog::printLastErrUnsafe()
{
    std::cout << "Last Error:" << errors.back() << " Time:" << timeStamps.back() << " ms" << "\n";
}

///Print  the last error in Thread Safe manner.
void ErrorLog::printLastErr()
{
    std::lock_guard<std::mutex> lock(logLock); //Lock is released when leaves scope
    printLastErr();
}

void ErrorLog::setConsoleOutput(bool flag)
{
    consoleOutput = flag;
}

