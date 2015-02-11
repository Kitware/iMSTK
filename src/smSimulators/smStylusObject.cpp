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

#include "smSimulators/smStylusObject.h"
#include "smCollision/smCollisionModel.h"
#include "smCollision/smSurfaceTree.h"
#include "smCollision/smOctreeCell.h"
#include "smMesh/smMesh.h"
#include "smUtilities/smGLUtils.h"

void smStylusRigidSceneObject::draw(smDrawParam p_params)
{
    smDouble matrix[16];
    smDouble matrixTransRot[16];
    smMatrix44<smDouble> temp;

#pragma unroll

    for (smInt i = 0; i < 2; i++)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        tree<smMeshContainer*>::pre_order_iterator iter = meshes.begin();
        glPushMatrix();

        if (i == 2 && enableDeviceManipulatedTool)
        {
            temp = iter.node->data->currentDeviceMatrix;
        }
        else
        {
            temp = iter.node->data->currentViewerMatrix;
        }

        temp.getMatrixForOpenGL(matrix);
        glMultMatrixd(matrix);
        glCallList(iter.node->data->mesh->renderingID);
        glPopMatrix();
        iter++;

        while (iter != meshes.end())
        {
            glPushMatrix();

            if (i == 2 && enableDeviceManipulatedTool)
            {
                temp = iter.node->data->currentDeviceMatrix;
            }
            else
            {
                temp = iter.node->data->currentViewerMatrix;
            }

            temp.getMatrixForOpenGL(matrix);
            glMultMatrixd(matrix);
            glCallList(iter.node->data->mesh->renderingID);
            glPopMatrix();
            iter++;
        }

        glPopMatrix();
    }
}

void smStylusRigidSceneObject::initDraw(smDrawParam p_params)
{
    smChar errorText[500];
    p_params.caller = this;
    tree<smMeshContainer*>::pre_order_iterator iter = meshes.begin();
    smGLInt newList = glGenLists(meshes.size());
    smGLUtils::queryGLError(errorText);

    smInt listCounter = 0;

    while (iter != meshes.end())
    {
        glNewList(newList + listCounter, GL_COMPILE);
        iter.node->data->mesh->draw(p_params);
        glEndList();
        iter.node->data->mesh->renderingID = (newList + listCounter);
        listCounter++;
        iter++;
    }
}

smStylusSceneObject::smStylusSceneObject(smErrorLog *p_log)
{
    type = SIMMEDTK_SMSTYLUSSCENEOBJECT;
    toolEnabled = true;
}

smStylusRigidSceneObject::smStylusRigidSceneObject(smErrorLog *p_log)
{
    type = SIMMEDTK_SMSTYLUSRIGIDSCENEOBJECT;
    updateViewerMatrixEnabled = true;
    rootIterator = meshes.begin();
    posCallBackEnabledForEntireObject = false;
    enableDeviceManipulatedTool = false;
}

smStylusDeformableSceneObject::smStylusDeformableSceneObject(smErrorLog *p_log)
{
    type = SIMMEDTK_SMSTYLUSDEFORMABLESCENEOBJECT;
}

smMeshContainer *smStylusRigidSceneObject::getMeshContainer(QString p_string) const
{
    tree<smMeshContainer*>::iterator iter = meshes.begin_leaf();

    while (iter != meshes.end_leaf())
    {
        if (iter.node->data->name == p_string)
        {
            return (*iter);
        }

        iter++;
    }
}

void smStylusRigidSceneObject::posTraverseCallBack(smMeshContainer &p_container)
{
    if (p_container.colModel != NULL)
    {
        p_container.colModel->transRot = p_container.currentMatrix;
        p_container.colModel->translateRot();
    }
}
