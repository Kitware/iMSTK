/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkCollisionData.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
CollisionDetectionAlgorithm::CollisionDetectionAlgorithm() : GeometryAlgorithm(),
    m_colData(std::make_shared<CollisionData>())
{
    setNumInputPorts(2);
}

bool
CollisionDetectionAlgorithm::areInputsValid()
{
    // We allow subclasses to not use input requirements
    if (m_requiredTypeChecks.size() == 0)
    {
        return true;
    }
    // Otherwise there must be 2 input ports for CD
    if (getInput(0) == nullptr)
    {
        LOG(WARNING) << "CollisionDetectionAlgorithm missing input A/input 0";
        return false;
    }
    if (getInput(1) == nullptr)
    {
        LOG(WARNING) << "CollisionDetectionAlgorithm missing input B/input 1";
        return false;
    }

    // Now type check, CollisionDetection allows reversible inputs
    GeometryCheck req1 = m_requiredTypeChecks.at(0);
    GeometryCheck req2 = m_requiredTypeChecks.at(1);

    if ((req1(getInput(0).get()) && req2(getInput(1).get()))
        || (req2(getInput(0).get()) && req1(getInput(1).get())))
    {
        return true;
    }
    else
    {
        LOG(WARNING) << "CollisionDetectionAlgorithm invalid input types. Received: ("
                     << getInput(0)->getTypeName() << ", " << getInput(1)->getTypeName() << ")";
        return false;
    }
}

void
CollisionDetectionAlgorithm::requestUpdate()
{
    // Determine if the input is flipped
    GeometryCheck req1 = m_requiredTypeChecks.at(0);
    GeometryCheck req2 = m_requiredTypeChecks.at(1);
    if ((req1(getInput(0).get()) && req2(getInput(1).get()))
        || (req2(getInput(0).get()) && req1(getInput(1).get())))
    {
        // Determine if the input is flipped so we can flip the output
        if ((req1(getInput(0).get()) && req2(getInput(1).get())))
        {
            m_flipOutput = false;
        }
        else if ((req2(getInput(0).get()) && req1(getInput(1).get())))
        {
            m_flipOutput = true;
        }
    }

    std::shared_ptr<Geometry> geomA = m_colData->geomA = getInput(0);
    std::shared_ptr<Geometry> geomB = m_colData->geomB = getInput(1);

    std::vector<CollisionElement>* a = &m_colData->elementsA;
    std::vector<CollisionElement>* b = &m_colData->elementsB;

    a->resize(0);
    b->resize(0);

    bool genA = m_generateCD_A;
    bool genB = m_generateCD_B;

    // If the input geometry is flipped, pass the element vectors and geometry
    // in flipped so the subclass doesn't have to worry about implementing flipping
    // and can assume A=type1 and B=type2
    // Ex: PlaneToSphere has A=Plane, B=Sphere, if user provides flipped input PlaneToSphere
    // does not need to worry about if A is sphere and B is plane instead
    if (m_flipOutput)
    {
        // Locally swap, output will still be in the order given by input
        std::swap(a, b);
        std::swap(geomA, geomB);
        std::swap(genA, genB);
    }

    // If user asked for both A and B
    if (genA && genB)
    {
        // If not implemented it will call A and B separately
        computeCollisionDataAB(geomA, geomB, *a, *b);
    }
    else
    {
        // If user asked for just A or just B
        if (genA)
        {
            computeCollisionDataA(geomA, geomB, *a);
        }
        if (genB)
        {
            computeCollisionDataB(geomA, geomB, *b);
        }

        // If asked for A or B but not implemented try AB
        if ((genA && !m_computeColDataAImplemented)
            || (genB && !m_computeColDataBImplemented))
        {
            computeCollisionDataAB(geomA, geomB, *a, *b);
        }
    }
}
} // namespace imstk