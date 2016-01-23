// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef IO_Config_H
#define IO_Config_H

#include "Core/Config.h"

/// NOTE: Make sure that this file is included only once per application.

#define IMSTK_REGISTER_IO_DELEGATES() \
    IMSTK_RUN_LOADER(register_IOMesh3dsDelegate); \
    IMSTK_RUN_LOADER(register_IOMeshAssimpDelegate); \
    IMSTK_RUN_LOADER(register_IOMeshVegaDelegate); \
    IMSTK_RUN_LOADER(register_IOMeshVTKDelegate);

void InitIODelegates();

#endif // IOConfig_H
