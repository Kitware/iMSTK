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

#ifndef CURVEDGRASPER_H
#define CURVEDGRASPER_H

#include "smSimulators/smStylusObject.h"
#include "smMesh/smSurfaceMesh.h"
/// \brief Cruver Grasper tool
class curvedGrasper: public smStylusRigidSceneObject
{
public:
    /// \brief phantom device ID that will be listened
    smInt phantomID;
    /// \brief buttons states of haptic device
    smBool buttonState[2];

    /// \brief angle of the jaws
    smFloat angle;
    /// \brief maximum angle that jaws can open
    smFloat maxangle;

    /// \brief the pivto mesh container
    smMeshContainer meshContainer_pivot;
    /// \brief lower jaw container
    smMeshContainer meshContainer_lowerJaw;

    /// \brief upper jaw container
    smMeshContainer meshContainer_upperJaw;

    /// \brief stores the pivot mesh
    smSurfaceMesh *mesh_pivot;
    /// \brief stores lower jaw mesh
    smSurfaceMesh *mesh_lowerJaw;
    /// \brief stores upper mesh jaw
    smSurfaceMesh *mesh_upperJaw;

    /// \brief constrcutor that gest hatpic device ID (e.g. 0 or 1), pivot, lower and upper mesh file names
    curvedGrasper(smInt ID,
                  smChar * p_pivotModelFileName = "../../resources/models/curved_pivot.3DS",
                  smChar *p_lowerModelFileName = "../../resources/models/curved_upper.3DS",
                  smChar *p_upperModelFileName = "../../resources/models/curved_lower.3DS");

    /// \brief for jaw interface
    //smPipeRegisteration ADUpipeReg;
#ifdef smNIUSB6008DAQ
    smPipeRegisteration NIUSB6008pipeReg;
#endif
    /// \brief god object position
    smVec3d godPos;
    /// \brief  god object matrix
    smFloat godMat[9];
    /// \brief  interface for DAQ
    smInt DAQdataID;
    /// \brief read  min  data value
    smFloat minValue;
    /// \brief read  max  data value
    smFloat maxValue;
    /// \brief 1/range value
    smFloat invRange;

public:
    /// \brief rendering the curved grasper
    void draw(smDrawParam p_params);
    /// \brief event handler
    void handleEvent(smEvent *p_event);
    /// \brief for open and close motion
    void updateOpenClose();
};

#endif
