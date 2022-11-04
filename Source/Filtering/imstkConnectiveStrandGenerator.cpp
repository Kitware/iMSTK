/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkConnectiveStrandGenerator.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkVecDataArray.h"

#include <random>

namespace imstk
{
ConnectiveStrandGenerator::ConnectiveStrandGenerator()
{
    setNumInputPorts(2);
    setRequiredInputType<SurfaceMesh>(0);
    setRequiredInputType<SurfaceMesh>(1);

    setNumOutputPorts(1);
    setOutput(std::make_shared<LineMesh>());
}

void
ConnectiveStrandGenerator::setInputMeshes(
    std::shared_ptr<SurfaceMesh> inputMeshA,
    std::shared_ptr<SurfaceMesh> inputMeshB)
{
    setInput(inputMeshA, 0);
    setInput(inputMeshB, 1);
}

std::shared_ptr<LineMesh>
ConnectiveStrandGenerator::getOutputMesh() const
{
    return std::static_pointer_cast<LineMesh>(getOutput(0));
}

void
ConnectiveStrandGenerator::requestUpdate()
{
    // Unpacking meshes
    std::shared_ptr<SurfaceMesh> meshA = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    std::shared_ptr<SurfaceMesh> meshB = std::dynamic_pointer_cast<SurfaceMesh>(getInput(1));

    // RNG for selecting faces to connect
    std::random_device              rd;                                     // obtain a random number from hardware
    std::mt19937                    gen(rd());                              // seed the generator
    std::uniform_int_distribution<> faceDistr(0, meshB->getNumCells() - 1); // define the range over cells of mesh B

    // Connective tissue geometry
    auto connectiveLineMesh = std::make_shared<LineMesh>();

    // Storage for connectivity of line mesh
    auto lineMeshVerticesPtr = std::make_shared<VecDataArray<double, 3>>();
    auto lineMeshIndicesPtr  = std::make_shared<VecDataArray<int, 2>>();

    // Verify normals are up to date
    meshA->computeTrianglesNormals();
    meshB->computeTrianglesNormals();

    // Create storage for new points
    // accessed by [cell_id, newPtOnCell_id]
    std::vector<std::vector<Vec3d>> sideApts;
    std::vector<std::vector<Vec3d>> sideBpts;

    // Generate random points on the faces of mesh A
    sideApts = generateRandomPointsOnMesh(meshA);

    // Generate random points on the faces of mesh B
    sideBpts = generateRandomPointsOnMesh(meshB);

    // Filter faces on mesh a to remove those facing away from mesh B
    // Checks nearest faces, if nearest face normal points in same general direction then ignore
    std::vector<int> meshAFiltered;
    for (int cell_idA = 0; cell_idA < meshA->getNumCells(); cell_idA++)
    {
        // Find nearest cell center on mesh B
        int    nearestId = -1;
        double nearestDistSquared = IMSTK_DOUBLE_MAX;

        Vec3d cellACenter = (meshA->getVertexPosition(meshA->getCells()->at(cell_idA)[0])
                             + meshA->getVertexPosition(meshA->getCells()->at(cell_idA)[1])
                             + meshA->getVertexPosition(meshA->getCells()->at(cell_idA)[2])) / 3.0;

        for (int cell_idB = 0; cell_idB < meshB->getNumCells(); cell_idB++)
        {
            Vec3d cellBCenter = (meshB->getVertexPosition(meshB->getCells()->at(cell_idB)[0])
                                 + meshB->getVertexPosition(meshB->getCells()->at(cell_idB)[1])
                                 + meshB->getVertexPosition(meshB->getCells()->at(cell_idB)[2])) / 3.0;

            double distSquared = (cellBCenter - cellACenter).squaredNorm();

            if (distSquared < nearestDistSquared)
            {
                nearestDistSquared = distSquared;
                nearestId = cell_idB;
            }
        }

        //Check the normal of the nearest cell to verify facing towards each other
        double dotCheck = meshA->getCellNormals()->at(cell_idA).dot(meshB->getCellNormals()->at(nearestId));
        if (dotCheck < -0.1)
        {
            meshAFiltered.push_back(cell_idA);
        }
    } // end loop over faces on mesh A

    // Generate strands of connective tissue between meshA and meshB
    // Loop over filtered subset of mesh A
    for (int cell_idA = 0; cell_idA < meshAFiltered.size(); cell_idA++)
    {
        // Turn the float strands per face into an int count
        // the fractional part turns into a chance to have an "extra"
        // strand on the triangle
        int    strandCount = static_cast<int>(m_strandsPerFace);
        double remainder   = m_strandsPerFace - strandCount;

        if (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) < remainder)
        {
            strandCount++;
        }

        // Loop over generated random points in this cell
        for (int surfNodeId = 0; surfNodeId < strandCount; surfNodeId++)
        {
            const Vec3d positionOnA = sideApts[meshAFiltered[cell_idA]][surfNodeId];
            Vec3d       positionOnB = Vec3d::Zero();

            // Get index of cell on B that creates a strand that does not penetrate mesh B
            while (true)
            {
                int sideBindx = faceDistr(gen);
                positionOnB = sideBpts[sideBindx][surfNodeId];

                // Check that direction is not inside of mesh B
                Vec3d  directionBA = (positionOnA - positionOnB).normalized();
                double dotCheck    = meshB->getCellNormals()->at(sideBindx).dot(directionBA);
                if (dotCheck > 0.1)
                {
                    break;
                }
            }

            Vec3d stepVec = (positionOnB - positionOnA) / static_cast<double>(m_segmentsPerStrand);

            int strandStartIndex = lineMeshVerticesPtr->size();
            for (int i = 0; i < m_segmentsPerStrand + 1; i++)
            {
                lineMeshVerticesPtr->push_back(positionOnA + static_cast<double>(i) * stepVec);
            }
            for (int i = 0; i < m_segmentsPerStrand; ++i)
            {
                lineMeshIndicesPtr->push_back(Vec2i(strandStartIndex + i, strandStartIndex + i + 1));
            }
        }
    } // end loop over cells in mesh A

    // Initialize line mesh
    connectiveLineMesh->initialize(lineMeshVerticesPtr, lineMeshIndicesPtr);
    setOutput(connectiveLineMesh, 0);
}

std::vector<std::vector<Vec3d>>
ConnectiveStrandGenerator::generateRandomPointsOnMesh(std::shared_ptr<SurfaceMesh>& mesh)
{
    std::vector<std::vector<Vec3d>> newFacePts;
    // Generate random points on the faces of mesh
    for (int cellId = 0; cellId < mesh->getNumCells(); cellId++)
    {
        // This will generate the maximum number of point per face that we
        // might need not all of these will be used
        int strandCount = static_cast<int>(m_strandsPerFace + 0.5);

        for (int ptId = 0; ptId < strandCount; ptId++)
        {
            newFacePts.push_back(std::vector<Vec3d>());

            Vec3d newPt = generateRandomPointOnFace(
                mesh->getVertexPosition(mesh->getCells()->at(cellId)[0]),
                mesh->getVertexPosition(mesh->getCells()->at(cellId)[1]),
                mesh->getVertexPosition(mesh->getCells()->at(cellId)[2]));

            newFacePts[cellId].push_back(newPt);
        }
    }
    return newFacePts;
}

const Vec3d
ConnectiveStrandGenerator::generateRandomPointOnFace(Vec3d& ptA, Vec3d& ptB, Vec3d& ptC)
{
    float r0 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float r1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    Vec3d newPt = (1.0 - sqrt(r0)) * ptA + (sqrt(r0) * (1 - r1)) * ptB + (r1 * sqrt(r0)) * ptC;
    return newPt;
}
} // namespace imstk