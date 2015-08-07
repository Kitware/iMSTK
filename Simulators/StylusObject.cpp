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

#include "Simulators/StylusObject.h"
#include "Collision/SurfaceTree.h"
#include "Collision/OctreeCell.h"
#include "Core/BaseMesh.h"
#include "Rendering/GLUtils.h"

StylusSceneObject::StylusSceneObject(std::shared_ptr<ErrorLog>/*p_log*/) : SceneObject()
{
    type = core::ClassType::StylusSceneObject;
    toolEnabled = true;
    this->setRenderDelegate(
      Factory<RenderDelegate>::createConcreteClass(
        "StylusRenderDelegate"));
}

StylusRigidSceneObject::StylusRigidSceneObject(std::shared_ptr<ErrorLog>/*p_log*/)
{
    type = core::ClassType::StylusRigidSceneObject;
    updateViewerMatrixEnabled = true;
    rootIterator = meshes.begin();
    posCallBackEnabledForEntireObject = false;
    enableDeviceManipulatedTool = false;
}

StylusDeformableSceneObject::StylusDeformableSceneObject(
                                            std::shared_ptr<ErrorLog>/*p_log*/)
                                            : StylusSceneObject()
{
    type = core::ClassType::StylusSeformableSceneObject;
}

MeshContainer *StylusRigidSceneObject::getMeshContainer(std::string p_string) const
{
    tree<MeshContainer*>::iterator iter = meshes.begin_leaf();

    while (iter != meshes.end_leaf())
    {
        if (iter.node->data->name == p_string)
        {
            return (*iter);
        }

        iter++;
    }
    return nullptr;
}

void StylusRigidSceneObject::posTraverseCallBack(MeshContainer &p_container)
{
    if (p_container.colModel != nullptr)
    {
        p_container.colModel->transRot = p_container.currentMatrix;
        p_container.colModel->translateRot();
    }
}

MeshContainer::MeshContainer( std::string p_name )
{
    name = p_name;
    offsetRotX = 0.0;
    offsetRotY = 0.0;
    offsetRotZ = 0.0;
    preOffsetPos = core::Vec3d::Zero();
    posOffsetPos = core::Vec3d::Zero();
    mesh = nullptr;
    colModel = nullptr;
}

MeshContainer::MeshContainer( std::string p_name, Mesh */*p_mesh*/, core::Vec3d p_prePos, core::Vec3d p_posPos, float p_offsetRotX, float p_offsetRotY, float p_offsetRotZ )
{
    offsetRotX = p_offsetRotX;
    offsetRotY = p_offsetRotY;
    offsetRotZ = p_offsetRotZ;
    preOffsetPos = p_prePos;
    posOffsetPos = p_posPos;
    name = p_name;
    colModel = nullptr;
}

void MeshContainer::computeCurrentMatrix()
{
    double pipi = 6.28318530717959;
    Eigen::Affine3d preTranslate( Eigen::Translation3d( preOffsetPos[0], preOffsetPos[1], preOffsetPos[2] ) );
    Eigen::Affine3d posTranslate( Eigen::Translation3d( posOffsetPos[0], posOffsetPos[1], posOffsetPos[2] ) );
    Eigen::Affine3d rx( Eigen::Affine3d( Eigen::AngleAxisd( pipi * offsetRotX, core::Vec3d::UnitX() ) ) );
    Eigen::Affine3d ry( Eigen::Affine3d( Eigen::AngleAxisd( pipi * offsetRotY, core::Vec3d::UnitY() ) ) );
    Eigen::Affine3d rz( Eigen::Affine3d( Eigen::AngleAxisd( pipi * offsetRotZ, core::Vec3d::UnitZ() ) ) );

    Matrix44d transform = ( preTranslate * rx * ry * rz * posTranslate ).matrix();
    tempCurrentMatrix *= transform;
    tempCurrentDeviceMatrix *= transform;
}

StylusPoints::StylusPoints()
{
    point = core::Vec3d::Zero();
    container = nullptr;
}

void StylusSceneObject::serialize( void */*p_memoryBlock*/ )
{
}

void StylusSceneObject::unSerialize( void */*p_memoryBlock*/ )
{
}

void StylusSceneObject::handleEvent(std::shared_ptr<core::Event>/*p_event*/ ) {}
void StylusRigidSceneObject::posTraverseCallBack()
{
}

tree< MeshContainer * >::iterator StylusRigidSceneObject::addMeshContainer( MeshContainer *p_meshContainer )
{
    tree<MeshContainer *>::iterator iter;

    if ( meshes.size() > 1 )
    {
        iter = meshes.append_child( rootIterator, p_meshContainer );
    }

    else
    {
        iter = meshes.insert( rootIterator, p_meshContainer );
    }

    indexIterators[p_meshContainer->name] = iter;
    return iter;
}

bool StylusRigidSceneObject::addMeshContainer( std::string p_ParentName, MeshContainer *p_meshContainer )
{
    tree<MeshContainer *>::iterator iter;

    if ( p_ParentName.size() > 0 )
    {
        if ( indexIterators.count( p_ParentName ) > 0 )
        {
            iter = indexIterators[p_ParentName];
            meshes.append_child( iter, p_meshContainer );
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

tree< MeshContainer * >::iterator StylusRigidSceneObject::addMeshContainer( tree< MeshContainer * >::iterator p_iterator, MeshContainer *p_meshContainer )
{
    return meshes.insert( p_iterator, p_meshContainer );
}

void StylusRigidSceneObject::handleEvent(std::shared_ptr<core::Event>/*p_event*/ ) {}

std::shared_ptr<SceneObject> StylusRigidSceneObject::clone()
{
    // WARNING: What is the purpose of this function
    std::shared_ptr<StylusRigidSceneObject> ret = std::make_shared<StylusRigidSceneObject>();
    return ret;
}

//void StylusSceneObject::init() {}
