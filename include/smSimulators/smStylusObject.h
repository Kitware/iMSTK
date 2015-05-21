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

#ifndef SMSTYLUSOBJECT_H
#define SMSTYLUSOBJECT_H

// STD includes
#include <unordered_map>

// Eigen includes
#include "Eigen/Geometry"

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smMesh/smMesh.h"
#include "smCore/smSceneObject.h"
#include "smUtilities/smMath.h"
#include "smExternal/tree.hh"

namespace smtk {
    namespace Event {
        class smEvent;
        class smEventHandler;
        class smCameraEvent;
    }
}

template<typename T> class smCollisionModel;
template<typename smSurfaceTreeCell> class smSurfaceTree;
class smOctreeCell;

/// \brief !!
class smMeshContainer
{
public:
    using SurfaceTreeType = smSurfaceTree<smOctreeCell>;

public:
    /// \brief constructor
    smMeshContainer(smString p_name = "");

    /// \brief constructor
    smMeshContainer(smString p_name, smMesh *p_mesh, smVec3d p_prePos, smVec3d p_posPos, smFloat p_offsetRotX, smFloat p_offsetRotY, smFloat p_offsetRotZ);

    void computeCurrentMatrix();

public:
    smString name;
    smFloat offsetRotX; // offset in rotation in x-direction
    smFloat offsetRotY; // offset in rotation in y-direction
    smFloat offsetRotZ; // offset in rotation in z-direction
    smVec3d preOffsetPos; // !!
    smVec3d posOffsetPos; // !!
    smMatrix44d accumulatedMatrix; // !!
    smMatrix44d accumulatedDeviceMatrix; // !!

    smMatrix44d currentMatrix; // !!
    smMatrix44d currentViewerMatrix; // !!
    smMatrix44d currentDeviceMatrix; // !!
    smMatrix44d tempCurrentMatrix; // !!
    smMatrix44d tempCurrentDeviceMatrix; // !!
    smMesh * mesh; // mesh
    std::shared_ptr<SurfaceTreeType> colModel; // octree of surface
};

/// \brief points on the stylus
struct smStylusPoints
{
    /// \brief constructor
    smStylusPoints();

    smVec3d point; // co-ordinates of points on stylus
    smMeshContainer *container; // !!
};

/// \brief stylus object of the scene (typically used for laparascopic VR simulations)
class smStylusSceneObject: public smSceneObject
{
public:
    /// \brief constructor
    smStylusSceneObject(std::shared_ptr<smErrorLog> p_log = nullptr);

    /// \brief !!
    virtual void serialize(void *p_memoryBlock) override;

    /// \brief !!
    virtual void unSerialize(void *p_memoryBlock) override;

    virtual void init() override;

    /// \brief handle the events such as button presses related to stylus
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;

public:
    smVec3d pos; // position of stylus
    smVec3d vel; // velocity of stylus
    smMatrix33d rot; // rotation of stylus
    smMatrix44d transRot; // !! translation and rotation matrix of stylus
    smMatrix44d transRotDevice; // translation and rotation matrix of devide controlling the stylus
    smBool toolEnabled; // !!

protected:
    std::shared_ptr<smtk::Event::smEventHandler> eventHandler;
};

/// \brief !!
class smStylusRigidSceneObject: public smStylusSceneObject
{
public:
    tree<smMeshContainer*> meshes; // meshes representing the stylus
    tree<smMeshContainer*>::iterator rootIterator; // !!
    volatile smBool updateViewerMatrixEnabled; // !!

    /// \brief to show the device tool..It is for debugging god object
    smBool enableDeviceManipulatedTool;

    /// \brief enabling post traverse callback enabled for each node.
    smBool posTraverseCallbackEnabled;

    /// \brief post traverse callback for each node
    virtual void posTraverseCallBack(smMeshContainer &p_container);

    /// \brief Post Traverse callback for the entire object.
    virtual void posTraverseCallBack();

    smBool posCallBackEnabledForEntireObject; // !!

    /// \brief !!
    smStylusRigidSceneObject(std::shared_ptr<smErrorLog> p_log = nullptr);

    /// \brief !!
    tree<smMeshContainer*>::iterator addMeshContainer(smMeshContainer *p_meshContainer);

    /// \brief !!
    smBool addMeshContainer(smString p_ParentName, smMeshContainer *p_meshContainer);

    /// \brief !!
    tree<smMeshContainer*>::iterator addMeshContainer(tree<smMeshContainer*>::iterator p_iterator, smMeshContainer *p_meshContainer);

    /// \brief !!
    smMeshContainer *getMeshContainer(smString p_string) const;

    virtual void handleEvent(std::shared_ptr<smtk::Event::smEvent> p_event) override;

    /// \brief !!
    std::shared_ptr<smSceneObject> clone() override;

    /// \brief !!
    virtual void initDraw(const smDrawParam &p_params) override;

    /// \brief !!
    virtual void draw(const smDrawParam &p_params) override;

    /// \brief !!
    virtual void init() override;

private:
    std::unordered_map<smString, tree<smMeshContainer*>::iterator> indexIterators;
};

/// \brief !!
class smStylusDeformableSceneObject: public smStylusSceneObject
{
    smStylusDeformableSceneObject(std::shared_ptr<smErrorLog> p_log = nullptr);
};

#endif
