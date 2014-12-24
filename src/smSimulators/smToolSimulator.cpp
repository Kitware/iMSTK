#include "smSimulators/smToolSimulator.h"

void smToolSimulator::updateTool(smStylusRigidSceneObject *p_tool)
{
    smMatrix44<smDouble> tempMat, tempMatDevice;
    smMatrix44<smDouble> mat;
    tree<smMeshContainer*>::pre_order_iterator iter = p_tool->meshes.begin();
    //update the Root node first
    iter.node->data->computeCurrentMatrix();
    tempMat = p_tool->transRot * iter.node->data->tempCurrentMatrix;
    iter.node->data->currentMatrix = tempMat;

    tempMatDevice = p_tool->transRotDevice * iter.node->data->tempCurrentDeviceMatrix;
    iter.node->data->currentDeviceMatrix = tempMatDevice;

    if (p_tool->posTraverseCallbackEnabled && p_tool->updateViewerMatrixEnabled)
    {
        iter.node->data->currentViewerMatrix = tempMat;
        p_tool->posTraverseCallBack(**iter);
    }
    else
    {
        p_tool->posTraverseCallBack(**iter);
    }

    iter.node->data->currentDeviceMatrix = tempMatDevice;
    iter++;

    while (iter != p_tool->meshes.end())
    {
        ///the parent nodes matrix should be accumulated
        iter.node->data->accumulatedMatrix = iter.node->parent->data->currentMatrix;
        iter.node->data->accumulatedDeviceMatrix = iter.node->parent->data->currentDeviceMatrix;
        iter.node->data->computeCurrentMatrix();
        iter.node->data->currentMatrix = iter.node->data->tempCurrentMatrix;
        iter.node->data->currentDeviceMatrix = iter.node->data->tempCurrentDeviceMatrix;

        if (p_tool->posTraverseCallbackEnabled && p_tool->updateViewerMatrixEnabled)
        {
            iter.node->data->currentViewerMatrix = iter.node->data->currentMatrix;
            p_tool->posTraverseCallBack(**iter);
        }
        else
        {
            p_tool->posTraverseCallBack(**iter);
        }

        iter++;
    }

    if (p_tool->posCallBackEnabledForEntireObject)
    {
        p_tool->posTraverseCallBack();
    }
}