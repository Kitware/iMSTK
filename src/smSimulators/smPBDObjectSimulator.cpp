/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */
 
#include "smSimulators/smPBDSceneObject.h"
#include "smSimulators/smPBDObjectSimulator.h"
#include "smRendering/smGLRenderer.h"

void smPBDObjectSimulator::draw(smDrawParam p_params)
{
    smObjectSimulator::draw(p_params);
    smPBDSurfaceSceneObject *sceneObject;

    for (smInt i = 0; i < objectsSimulated.size(); i++)
    {
        sceneObject = (smPBDSurfaceSceneObject*)objectsSimulated[i];
        smGLRenderer::draw(sceneObject->mesh->aabb);
    }
}