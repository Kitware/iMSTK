/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkLogger.h"

#include <gtest/gtest.h>
#include <vtksys/SystemTools.hxx>

int
main(int argc, char** argv)
{
    bool removeLog = true;
    for (int i = 0; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--imstk_keep_log")
        {
            removeLog = false;
        }
    }

    if (removeLog)
    {
        vtksys::SystemTools::RemoveADirectory("log");
    }

    vtksys::SystemTools::MakeDirectory("log");

    auto& logger = imstk::Logger::getInstance();
    logger.addFileSink("test", "log");

    ::testing::InitGoogleTest(&argc, argv);
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    return RUN_ALL_TESTS();
}