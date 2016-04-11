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

#include "imstkTetraTriangleMap.h"

namespace imstk
{

void
TetraTriangleMap::computeMap()
{
    if (!m_master || !m_slave)
    {
        LOG(WARNING) << "TetraTriangle map is being applied without valid geometries\n";
        return;
    }

    // Proceed to generate the map
    auto tetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh> (m_master);
    auto triMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh> (m_slave);

    weightsArray weights;
    int numSurfaceVertices = triMesh->getNumVertices();
    int numTetrahedra = tetMesh->getNumTetrahedra();

    for (int i = 0; i < numSurfaceVertices; ++i)
    {
        imstk::Vec3d surfVertPos = triMesh->getVertexInitialPosition(i);

        // find the enclosing element
        int closestEle = findEclosingTetrahedra(tetMesh, surfVertPos);

        // if not inside of any element,
        // find the tetrahedra whose centroid is the closest
        if (closestEle < 0)
        {
            closestEle = findClosestTetrahedra(tetMesh, surfVertPos);
        }

        // compute the weights
        tetMesh->computeBarycentricWeights(closestEle, surfVertPos, weights);

        m_enclosingTetra.push_back(closestEle);// store nearest tetrahedra
        m_weights.push_back(weights);// store weights
    }
}

int
TetraTriangleMap::findClosestTetrahedra(const std::shared_ptr<imstk::TetrahedralMesh> tetraMesh, const imstk::Vec3d& p)
{
    // search
    double closestDistance = std::numeric_limits<double>::max();
    int closestTetrahedra = -1;
    for (int t = 0; t < tetraMesh->getNumTetrahedra(); ++t)
    {
        imstk::Vec3d center(0, 0, 0);
        auto vert = tetraMesh->getTetrahedronVertices(t);
        for (int i = 0; i < 4; ++i)
        {
            center += tetraMesh->getInitialVertexPosition(vert[i]);
        }

        double dist = (p - center).norm();
        if (dist < closestDistance)
        {
            closestDistance = dist;
            closestTetrahedra = t;
        }
    }

    return closestTetrahedra;
}

int
TetraTriangleMap::findEclosingTetrahedra(const std::shared_ptr<imstk::TetrahedralMesh> tetraMesh, const imstk::Vec3d& p)
{
    imstk::Vec3d boundingBoxMin;
    imstk::Vec3d boundingBoxMax;
    std::vector<int> probables;

    // Eliminate the improbables based in bounding box test
    for (int t = 0; t < tetraMesh->getNumTetrahedra(); ++t)
    {
        tetraMesh->computeTetrahedraBoundingBox(boundingBoxMin, boundingBoxMax, t);

        if ((p[0] >= boundingBoxMin[0] && p[0] <= boundingBoxMax[0]) &&
            (p[1] >= boundingBoxMin[1] && p[1] <= boundingBoxMax[1]) &&
            (p[2] >= boundingBoxMin[2] && p[2] <= boundingBoxMax[2]))
        {
            probables.push_back(t);
        }
    }

    // Check which probable tetrahedra the point belongs to
    int elclosingTetra = -1;
    weightsArray weights;
    for (auto it = probables.begin(); it != probables.end(); ++it)
    {
        tetraMesh->computeBarycentricWeights(*it, p, weights);

        if ((weights[0] >= 0) && (weights[1] >= 0) && (weights[2] >= 0) && (weights[3] >= 0))
        {
            elclosingTetra = *it;
            break;
        }
    }
    return elclosingTetra;
}

void
TetraTriangleMap::setMaster(std::shared_ptr<Geometry> master)
{
    if (master->getType() == imstk::GeometryType::TetrahedralMesh)
    {
        m_master = master;
    }
    else
    {
        LOG(WARNING) << "The geometry provided is not of tetrahedral type\n";
    }
}

void
TetraTriangleMap::setSlave(std::shared_ptr<Geometry> slave)
{
    if (slave->getType() == imstk::GeometryType::SurfaceMesh)
    {
        m_slave = slave;
    }
    else
    {
        LOG(WARNING) << "The geometry provided is not of surface triangular type\n";
    }

}

void
TetraTriangleMap::applyMap()
{
    if (m_isActive)
    {
        if (!m_master || !m_slave)
        {
            LOG(WARNING) << "TetraTriangle map is not completely defined!\n";
            return;
        }

        auto tetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh> (m_master);
        auto triMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh> (m_slave);

        imstk::Vec3d newPos;
        for (int v = 0; v < triMesh->getNumVertices(); ++v)
        {
            newPos.setZero();
            auto tetVerts = tetMesh->getTetrahedronVertices(m_enclosingTetra[v]);
            for (int i = 0; i < 4; ++i)
            {
                newPos += tetMesh->getInitialVertexPosition(tetVerts[i]) * m_weights.at(v)[i];
            }
            triMesh->setVertexPosition(v, newPos);
        }
    }
}
}