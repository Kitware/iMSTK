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

#ifndef SMCORECLASS_H
#define SMCORECLASS_H

// STL includes
#include <atomic>
#include <iostream>

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smRendering/smConfigRendering.h"

class smSDK;
class smCoreClass;
class smObjectSimulator;
class smViewer;

/// \brief  viewer sends this to all objects to be rendered
struct smDrawParam
{
    smViewer    *rendererObject;
    smCoreClass *caller;
    smGLFloat *viewMatrix; ///< Pointer to an array detailing a 4x4 OpenGL view matrix
    smGLFloat *projMatrix; ///< Pointer to an array detailing a 4x4 OpenGL projection matrix
    void *data;
};

/// \brief simulator calls object and sends this structure
struct smSimulationParam
{
    smObjectSimulator *objectSimulator;
    void *caller;
    void *data;

};

/// \brief  This class indicates the unified id of all  objects in the framework
struct smUnifiedID
{

private:
    /// \brief  atomic integer counter that is used to assign a unique number for  each object
    static std::atomic_int IDcounter;
    /// \brief  sdk ID. for network use
    smShort sdkID;

public:
    /// \brief  unique ID
    smShort ID;
    /// \brief   machine ID. for network use
    smShort machineID;

    /// \brief  constructor
    smUnifiedID();

    /// \brief  generate unique ID
    void generateUniqueID();

    /// \brief  returns SDK id
    smInt getSDKID() const;

    /// \brief  set with another smUnifiedID
    void operator =(const smUnifiedID &p_id);

    /// \brief comparison with another smUnifiedID
    bool operator==(smUnifiedID &p_id);

    /// \brief checks with ID. can be used to compare the object on the same machine
    bool operator==(smInt &p_ID);

    /// \brief comparison with id
    bool operator!=(smInt &p_ID);

    friend smSDK;
};

/// \brief core class of all objects in framework
class smCoreClass
{

protected:
    /// \brief class type
    smClassType type;
    /// \brief reference counter to identify the count the usage
    std::atomic_int referenceCounter;

public:
    /// \brief name of the class
    smString name;
    /// \brief unique ID
    smUnifiedID uniqueId;
    /// \brief renderDetail specifies visualization type
    smRenderDetail renderDetail;
    /// \brief draw order of the object
    smClassDrawOrder drawOrder;

    /// \brief constructor
    smCoreClass();

    /// \brief get type of the class
    smClassType getType() const ;

    /// \brief his function is called by the renderer. The p_params stores renderer pointers
    virtual void initDraw(const smDrawParam &p_params);

    /// \brief draw function is called for visualization the object
    virtual void draw(const smDrawParam &p_params);

    /// \brief initialization of simulation
    virtual void initSimulate(const smSimulationParam &p_params);

    /// \brief simulates the object
    virtual void simulate(const smSimulationParam &p_params);

    /// \brief print the object
    virtual void print() const;

    /// \brief set the name of object
    void setName(const smString &p_objectName);

    /// \brief get the name of the object
    smString getName() const;

    friend smSDK;
};

/// \brief for future use
class smInterface
{

};

#endif
