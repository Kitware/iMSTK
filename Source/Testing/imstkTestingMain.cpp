/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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

#ifndef iMSTK_SYNCHRONOUS_LOGGING
    auto& logger = imstk::Logger::getInstance();
    logger.addFileSink("test", "log");
#endif

    ::testing::InitGoogleTest(&argc, argv);
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    return RUN_ALL_TESTS();
}