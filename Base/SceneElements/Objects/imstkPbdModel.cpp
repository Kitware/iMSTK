#include "imstkPbdModel.h"
#include "imstkTetrahedralMesh.h"
#include "imstkSurfaceMesh.h"

#include <g3log/g3log.hpp>

namespace imstk {

bool PositionBasedModel::initFEMConstraints(FEMConstraint::MaterialType type)
{
    // check if constraint type matches the mesh type
    if (m_mesh->getType() != Geometry::Type::TetrahedralMesh)
    {
        LOG(WARNING) << "FEM Tetrahedral constraint should come with tetrahedral mesh";
        return false;
    }
    // ok, now create constraints
    auto tetMesh = static_cast<TetrahedralMesh*>(m_mesh);
    std::vector<TetrahedralMesh::TetraArray> elements = tetMesh->getTetrahedraVertices();
    for (int k = 0; k < elements.size(); ++k) {
        TetrahedralMesh::TetraArray& tet = elements[k];
        FEMTetConstraint* c = new FEMTetConstraint(type);
        c->initConstraint(*this, tet[0], tet[1], tet[2], tet[3]);
        m_constraints.push_back(c);
    }
    return true;
}

bool PositionBasedModel::initVolumeConstraints(const double &stiffness)
{
    // check if constraint type matches the mesh type
    if (m_mesh->getType() != Geometry::Type::TetrahedralMesh)
    {
        LOG(WARNING) << "Volume constraint should come with volumetric mesh";
        return false;
    }

    // ok, now create constraints
    auto tetMesh = static_cast<TetrahedralMesh*>(m_mesh);
    std::vector<TetrahedralMesh::TetraArray> elements = tetMesh->getTetrahedraVertices();
    for (int k = 0; k < elements.size(); ++k) {
        TetrahedralMesh::TetraArray& tet = elements[k];
        VolumeConstraint* c = new VolumeConstraint;
        c->initConstraint(*this, tet[0], tet[1], tet[2], tet[3], stiffness);
        m_constraints.push_back(c);
    }
    return true;
}

bool PositionBasedModel::initDistanceConstraints(const double &stiffness)
{
    if (m_mesh->getType() == Geometry::Type::TetrahedralMesh) {
        auto tetMesh = static_cast<TetrahedralMesh*>(m_mesh);
        int nV = tetMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));
        std::vector<TetrahedralMesh::TetraArray> elements = tetMesh->getTetrahedraVertices();
        for (int k = 0; k < elements.size(); ++k) {
            TetrahedralMesh::TetraArray& tet = elements[k];
            unsigned int i1;
            unsigned int i2;
            i1 = tet[0];
            i2 = tet[1];
            // check if added or not
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
            i1 = tet[1];
            i2 = tet[2];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
            i1 = tet[2];
            i2 = tet[0];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
            i1 = tet[0];
            i2 = tet[3];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
            i1 = tet[1];
            i2 = tet[3];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
            i1 = tet[2];
            i2 = tet[3];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
        }
    } else if (m_mesh->getType() == Geometry::Type::SurfaceMesh) {
        auto triMesh = static_cast<SurfaceMesh*>(m_mesh);
        int nV = triMesh->getNumVertices();
        std::vector<std::vector<bool>> E(nV, std::vector<bool>(nV, 1));
        std::vector<SurfaceMesh::TriangleArray> elements = triMesh->getTrianglesVertices();
        for (int k = 0; k < elements.size(); ++k) {
            SurfaceMesh::TriangleArray& tri = elements[k];
            unsigned int i1;
            unsigned int i2;
            i1 = tri[0];
            i2 = tri[1];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
            i1 = tri[1];
            i2 = tri[2];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }
            i1 = tri[2];
            i2 = tri[0];
            if (E[i1][i2] && E[i2][i1]) {
                DistanceConstraint* c = new DistanceConstraint;
                c->initConstraint(*this, i1, i2, stiffness);
                m_constraints.push_back(c);
                E[i1][i2] = 0;
            }

        }
    }
    return true;
}

bool PositionBasedModel::initAreaConstraints(const double &stiffness)
{
    // check if constraint type matches the mesh type
    if (m_mesh->getType() != Geometry::Type::SurfaceMesh)
    {
        LOG(WARNING) << "Area constraint should come with a triangular mesh";
        return false;
    }

    // ok, now create constraints
    auto triMesh = static_cast<SurfaceMesh*>(m_mesh);
    std::vector<SurfaceMesh::TriangleArray> elements = triMesh->getTrianglesVertices();
    for (int k = 0; k < elements.size(); ++k) {
        SurfaceMesh::TriangleArray& tri = elements[k];
        AreaConstraint* c = new AreaConstraint;
        c->initConstraint(*this, tri[0], tri[1], tri[2], stiffness);
        m_constraints.push_back(c);
    }
    return true;
}

bool PositionBasedModel::initDihedralConstraints(const double &stiffness)
{
    if (m_mesh->getType() != Geometry::Type::SurfaceMesh)
    {
        LOG(WARNING) << "Dihedral constraint should come with a triangular mesh";
        return false;
    }

    // ok, now create constraints
    auto triMesh = static_cast<SurfaceMesh*>(m_mesh);
    std::vector<SurfaceMesh::TriangleArray> elements = triMesh->getTrianglesVertices();
    // following algorithm is terrible, should use half-edge instead
    std::vector<std::vector<unsigned int>> onering(triMesh->getNumVertices());
    for (int k = 0; k < elements.size(); ++k) {
        SurfaceMesh::TriangleArray& tri = elements[k];
        onering[tri[0]].push_back(k);
        onering[tri[1]].push_back(k);
        onering[tri[2]].push_back(k);
    }
    std::vector<std::vector<bool>> E(triMesh->getNumVertices(), std::vector<bool>(triMesh->getNumVertices(), 1));
    for (int k = 0; k < elements.size(); ++k) {
        SurfaceMesh::TriangleArray& tri = elements[k];
        std::vector<unsigned int>& r1 = onering[tri[0]];
        std::vector<unsigned int>& r2 = onering[tri[1]];
        std::vector<unsigned int>& r3 = onering[tri[2]];
        std::sort(r1.begin(), r1.end());
        std::sort(r2.begin(), r2.end());
        std::sort(r3.begin(), r3.end());
        std::vector<unsigned int> rs;
        std::vector<unsigned int>::iterator it;
        // check if proccessed or not
        if (E[tri[0]][tri[1]] && E[tri[1]][tri[0]]) {
            rs.resize(2);
            it = std::set_intersection(r1.begin(), r1.end(), r2.begin(), r2.end(), rs.begin());
            rs.resize(it - rs.begin());
            if (rs.size() > 1) {
                int idx = (rs[0] == k)?1:0;
                SurfaceMesh::TriangleArray& t = elements[rs[idx]];
                for (int i = 0; i < 3; ++i) {
                    if (t[i] != tri[0] && t[i] != tri[1]) {
                        idx = i;
                        break;
                    }
                }

                DihedralConstraint* c = new DihedralConstraint;
                c->initConstraint(*this, tri[2], t[idx], tri[0], tri[1], stiffness);
                m_constraints.push_back(c);
            }
            E[tri[0]][tri[1]] = 0;
        }
        if (E[tri[1]][tri[2]] && E[tri[2]][tri[1]]) {
            rs.resize(2);
            it = std::set_intersection(r2.begin(), r2.end(), r3.begin(), r3.end(), rs.begin());
            rs.resize(it - rs.begin());
            if (rs.size() > 1) {
                int idx = (rs[0] == k)?1:0;
                SurfaceMesh::TriangleArray& t = elements[rs[idx]];
                for (int i = 0; i < 3; ++i) {
                    if (t[i] != tri[1] && t[i] != tri[2]) {
                        idx = i;
                        break;
                    }
                }

                DihedralConstraint* c = new DihedralConstraint;
                c->initConstraint(*this, tri[0], t[idx], tri[1], tri[2], stiffness);
                m_constraints.push_back(c);
            }
            E[tri[1]][tri[2]] = 0;
        }
        if (E[tri[2]][tri[0]] && E[tri[0]][tri[2]]) {
            rs.resize(2);
            it = std::set_intersection(r3.begin(), r3.end(), r1.begin(), r1.end(), rs.begin());
            rs.resize(it - rs.begin());
            if (rs.size() > 1) {
                int idx = (rs[0] == k)?1:0;
                SurfaceMesh::TriangleArray& t = elements[rs[idx]];
                for (int i = 0; i < 3; ++i) {
                    if (t[i] != tri[2] && t[i] != tri[0]) {
                        idx = i;
                        break;
                    }
                }

                DihedralConstraint* c = new DihedralConstraint;
                c->initConstraint(*this, tri[1], t[idx], tri[2], tri[0], stiffness);
                m_constraints.push_back(c);
            }
            E[tri[2]][tri[0]] = 0;
        }
    }
    return true;
}

void PositionBasedModel::constraintProjection()
{
    int i = 0;
    while (++i < maxIter) {
        for (int k = 0; k < m_constraints.size(); ++k) {
            m_constraints[k]->solvePositionConstraint(*this);
        }
    }

}

void PositionBasedModel::updatePhysicsGeometry()
{
    for (int i = 0; i < m_mesh->getNumVertices(); ++i) {
        m_mesh->setVerticePosition(i, m_state->getVertexPosition(i));
    }
}

}
