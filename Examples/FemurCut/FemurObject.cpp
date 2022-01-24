///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/

#include "FemurObject.h"
#include "imstkImageData.h"
#include "imstkLevelSetModel.h"
#include "imstkLocalMarchingCubes.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkVisualModel.h"

FemurObject::FemurObject() : LevelSetDeformableObject("Femur"),
    m_isoExtract(std::make_shared<LocalMarchingCubes>())
{
    std::shared_ptr<ImageData> initLvlSetImage = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/legs/femurBoneSolid_SDF.nii")->cast(IMSTK_DOUBLE);
    //const Vec3d& currSpacing = initLvlSetImage->getSpacing();

    // Note: Anistropic scaling would invalidate the SDF
    initLvlSetImage->setOrigin(Vec3d(0.0, 0.8, 1.5));

    // Setup the Parameters
    imstkNew<LevelSetModelConfig> lvlSetConfig;
    lvlSetConfig->m_sparseUpdate = true;
    lvlSetConfig->m_substeps     = 15;

    // Too many chunks and you'll hit memory constraints quickly
    // Too little chunks and the updates for a chunk will take too long
    // The chunks must divide the image dimensions-1 (image dim-1 must be divisible by # chunks)
    m_isoExtract->setInputImage(initLvlSetImage);
    m_isoExtract->setIsoValue(0.0);
    m_isoExtract->setNumberOfChunks(Vec3i(32, 9, 9));
    m_isoExtract->update();

    if (m_useRandomChunkColors)
    {
        srand(static_cast<unsigned int>(time(NULL)));
    }

    createVisualModels();

    // Setup the Object
    imstkNew<SignedDistanceField> sdf(initLvlSetImage);

    // Setup the Model
    imstkNew<LevelSetModel> model;
    model->setModelGeometry(sdf);
    model->configure(lvlSetConfig);

    setPhysicsGeometry(sdf);
    setCollidingGeometry(sdf);
    setDynamicalModel(model);

    // Setup a custom task to forward the modified voxels of the level set model
    // to the marching cubes before they're cleared
    m_forwardModifiedVoxels = std::make_shared<TaskNode>(
        std::bind(&FemurObject::updateModifiedVoxels, this), "Isosurface: SetModifiedVoxels");
    m_taskGraph->addNode(m_forwardModifiedVoxels);
}

void
FemurObject::visualUpdate()
{
    // Update any chunks that contain a voxel which was set modified
    m_isoExtract->update();

    // Create meshes for chunks if they now contain vertices (and weren't already generated)
    // You could just create all the chunks, but this saves some memory for internal/empty ones
    createVisualModels();
}

void
FemurObject::createVisualModels()
{
    const Vec3i& numChunks = m_isoExtract->getNumberOfChunks();
    for (int i = 0; i < numChunks[0] * numChunks[1] * numChunks[2]; i++)
    {
        auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_isoExtract->getOutput(i));
        if (surfMesh->getNumVertices() > 0 && m_chunksGenerated.count(i) == 0)
        {
            imstkNew<VisualModel> surfMeshModel;
            surfMeshModel->setGeometry(m_isoExtract->getOutput(i));
            imstkNew<RenderMaterial> material;
            material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
            material->setLineWidth(4.0);
            if (m_useRandomChunkColors)
            {
                const double r = (rand() % 10000) / 10000.0;
                const double g = (rand() % 10000) / 10000.0;
                const double b = (rand() % 10000) / 10000.0;
                material->setColor(Color(r, g, b));
            }
            else
            {
                material->setColor(Color::Bone);
            }
            //material->setOpacity(0.7);
            surfMeshModel->setRenderMaterial(material);
            addVisualModel(surfMeshModel);
            m_chunksGenerated.insert(i);
        }
    }
}

void
FemurObject::updateModifiedVoxels()
{
    // Forward the level set's modified nodes to the isosurface extraction
    for (auto i : getLevelSetModel()->getNodesToUpdate())
    {
        m_isoExtract->setModified(std::get<0>(i.second));
    }
}

void
FemurObject::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Copy, sum, and connect the model graph to nest within this graph
    m_taskGraph->addEdge(source, getUpdateNode());

    m_dynamicalModel->initGraphEdges();
    m_taskGraph->nestGraph(m_dynamicalModel->getTaskGraph(), getUpdateNode(), getUpdateGeometryNode());

    // The levelsetmodel produces a list of modified voxels, we forward that to the isosurface extraction
    // filter to update only the modified chunks
    m_taskGraph->addEdge(getLevelSetModel()->getGenerateVelocitiesEndNode(), m_forwardModifiedVoxels);
    m_taskGraph->addEdge(m_forwardModifiedVoxels, getLevelSetModel()->getQuantityEvolveNode(0));

    m_taskGraph->addEdge(getUpdateGeometryNode(), sink);
}