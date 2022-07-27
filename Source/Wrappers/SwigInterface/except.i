/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. 
*/

%include "exception.i"

/*
 * List all functions that we want to enable C# or Unity to catch the exception
 * as all swig setting this needs to occur before the %include of the file that
 * contains the function definition, this will wrap an internal try/catch block
 * around the function call and in catch forwad the exception to a C# exception
 * handler
 */

%catches(std::exception, ...) imstk::Scene::advance;
%catches(std::exception, ...) imstk::Scene::initialize;
