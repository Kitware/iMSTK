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
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smUtilities/smMath.h"
#include "smExternal/tree.hh"


template<typename T> class smCollisionModel;
template<typename smSurfaceTreeCell> class smSurfaceTree;
struct smOctreeCell;

/// \brief !!
struct smMeshContainer
{
public:
    smString name;

    /// \brief constructor
    smMeshContainer(smString p_name = "")
    {
        name = p_name;
        offsetRotX = 0.0;
        offsetRotY = 0.0;
        offsetRotZ = 0.0;
        preOffsetPos = smVec3f::Zero();
        posOffsetPos = smVec3f::Zero();
        mesh = NULL;
        colModel = NULL;
    }

    /// \brief constructor
    smMeshContainer(smString p_name, smMesh *p_mesh, smVec3f p_prePos, smVec3f p_posPos, smFloat p_offsetRotX, smFloat p_offsetRotY, smFloat p_offsetRotZ)
    {
        offsetRotX = p_offsetRotX;
        offsetRotY = p_offsetRotY;
        offsetRotZ = p_offsetRotZ;
        preOffsetPos = p_prePos;
        posOffsetPos = p_posPos;
        name = p_name;
        colModel = NULL;
    }

    smFloat offsetRotX; ///< offset in rotation in x-direction
    smFloat offsetRotY; ///< offset in rotation in y-direction
    smFloat offsetRotZ; ///< offset in rotation in z-direction
    smVec3f preOffsetPos; ///< !!
    smVec3f posOffsetPos; ///< !!
    smMatrix44f accumulatedMatrix; ///< !!
    smMatrix44f accumulatedDeviceMatrix; ///< !!

    smMatrix44f currentMatrix; ///< !!
    smMatrix44f currentViewerMatrix; ///< !!
    smMatrix44f currentDeviceMatrix; ///< !!
    smMatrix44f tempCurrentMatrix; ///< !!
    smMatrix44f tempCurrentDeviceMatrix; ///< !!
    smMesh * mesh; ///< mesh
    smSurfaceTree<smOctreeCell> *colModel; ///< octree of surface

    /// \brief !!
    inline void computeCurrentMatrix()
    {
        Eigen::Affine3f preTranslate(Eigen::Translation3f(preOffsetPos[0],preOffsetPos[1], preOffsetPos[2]));
        Eigen::Affine3f posTranslate(Eigen::Translation3f(posOffsetPos[0], posOffsetPos[1], posOffsetPos[2]));
        Eigen::Affine3f rx(Eigen::Affine3f(Eigen::AngleAxisf(SM_PI_TWO * offsetRotX, smVec3f::UnitX())));
        Eigen::Affine3f ry(Eigen::Affine3f(Eigen::AngleAxisf(SM_PI_TWO * offsetRotY, smVec3f::UnitY())));
        Eigen::Affine3f rz(Eigen::Affine3f(Eigen::AngleAxisf(SM_PI_TWO * offsetRotZ, smVec3f::UnitZ())));

        smMatrix44f transform = (preTranslate * rx * ry *rz * posTranslate).matrix();
        tempCurrentMatrix *= transform;
        tempCurrentDeviceMatrix *= transform;
    }
};

/// \brief points on the stylus
struct smStylusPoints
{
    /// \brief constructor
    smStylusPoints()
    {
        point = smVec3f::Zero();
        container = NULL;
    }

    smVec3f point; ///< co-ordinates of points on stylus
    smMeshContainer *container; ///< !!
};

/// \brief stylus object of the scene (typically used for laparascopic VR simulations)
class smStylusSceneObject: public smSceneObject
{

public:
    smVec3f pos; ///< position of stylus
    smVec3f vel; ///< velocity of stylus
    smMatrix33d rot; ///< rotation of stylus
    smMatrix44f transRot; ///< !! translation and rotation matrix of stylus
    smMatrix44f transRotDevice; ///< translation and rotation matrix of devide controlling the stylus
    smBool toolEnabled; ///< !!

    /// \brief constructor
    smStylusSceneObject(smErrorLog *p_log = NULL);

    /// \brief !!
    virtual void serialize(void *p_memoryBlock)
    {
    }

    /// \brief !!
    virtual void unSerialize(void *p_memoryBlock)
    {
    }

    /// \brief handle the events such as button presses related to stylus
    virtual void handleEvent(smEvent *p_event) {};
};

/// \brief !!
class smStylusRigidSceneObject: public smStylusSceneObject, public smEventHandler
{
    std::unordered_map<smString, tree<smMeshContainer*>::iterator> indexIterators;
public:
    tree<smMeshContainer*> meshes; ///< meshes representing the stylus
    tree<smMeshContainer*>::iterator rootIterator; ///< !!
    volatile smBool updateViewerMatrixEnabled; ///< !!

    /// \brief to show the device tool..It is for debugging god object
    smBool enableDeviceManipulatedTool;

    /// \brief enabling post traverse callback enabled for each node.
    smBool posTraverseCallbackEnabled;

    /// \brief post traverse callback for each node
    virtual void posTraverseCallBack(smMeshContainer &p_container);

    /// \brief Post Traverse callback for the entire object.
    virtual void posTraverseCallBack()
    {
    }

    smBool posCallBackEnabledForEntireObject; ///< !!

    /// \brief !!
    smStylusRigidSceneObject(smErrorLog *p_log = NULL);

    /// \brief !!
    tree<smMeshContainer*>::iterator &addMeshContainer(smMeshContainer *p_meshContainer)
    {
        tree<smMeshContainer*>::iterator iter;

        if (meshes.size() > 1)
        {
            iter = meshes.append_child(rootIterator, p_meshContainer);
        }

        else
        {
            iter = meshes.insert(rootIterator, p_meshContainer);
        }

        indexIterators[p_meshContainer->name] = iter;
        return iter;
    }

    /// \brief !!
    smBool addMeshContainer(smString p_ParentName, smMeshContainer *p_meshContainer)
    {
        tree<smMeshContainer*>::iterator iter;

        if (p_ParentName.size() > 0)
        {
            if (indexIterators.count(p_ParentName) > 0)
            {
                iter = indexIterators[p_ParentName];
                meshes.append_child(iter, p_meshContainer);
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    /// \brief !!
    tree<smMeshContainer*>::iterator addMeshContainer(tree<smMeshContainer*>::iterator p_iterator, smMeshContainer *p_meshContainer)
    {
        return meshes.insert(p_iterator, p_meshContainer);
    }

    /// \brief !!
    smMeshContainer *getMeshContainer(smString p_string) const;

    virtual void handleEvent(smEvent *p_event) {};

    /// \brief !!
    smSceneObject *clone()
    {
        smStylusRigidSceneObject *ret = new smStylusRigidSceneObject();
        return ret;
    }

    /// \brief !!
    virtual void initDraw(smDrawParam p_params);

    /// \brief !!
    virtual void draw(smDrawParam p_params);
};

/// \brief !!
class smStylusDeformableSceneObject: public smStylusSceneObject
{
    smStylusDeformableSceneObject(smErrorLog *p_log = NULL);
};

#endif
