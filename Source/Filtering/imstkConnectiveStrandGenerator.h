/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkDataArray.h"

#include <random>
namespace imstk
{
class PointSet;
class SurfaceMesh;
class LineMesh;

///
/// \class ConnectiveStrandGenerator
///
/// \brief This filter takes in two surface meshes and generates
/// a LineMesh that connectes the faces of the two meshes. Internally
/// the strands are generated such that only faces with normals pointing
/// in opposite direction are used.  This cuts down on penetration.
/// This is expected to be used to generate connective tissue with the
/// ProximitySurfaceSelectoror any other method to select a subset of a
/// surface mesh to connect.
///
class ConnectiveStrandGenerator : public GeometryAlgorithm
{
public:
    ConnectiveStrandGenerator();
    ~ConnectiveStrandGenerator() override = default;

    void setInputMeshes(
        std::shared_ptr<SurfaceMesh> inputMeshA,
        std::shared_ptr<SurfaceMesh> inputMeshB);

    std::shared_ptr<LineMesh> getOutputMesh() const;

    ///
    /// \brief Get/Set the number of segments each connective strand is made of.
    /// Defaults to 3.
    ///@{
    void setSegmentsPerStrand(int numSegs) { m_segmentsPerStrand = numSegs; }
    int getSegmentsPerStrand() { return m_segmentsPerStrand; }
    ///@}

    ///
    /// \brief Get/Set the number of of connective strands attached to each face
    /// Defaults to 1.
    ///@{
    void setStrandsPerFace(double density) { m_strandsPerFace = density; }
    double getStrandsPerFace() { return m_strandsPerFace; }
///@}

protected:
    void requestUpdate() override;

    /// \brief Filter faces on meshA to remove those facing away from meshB
    /// Checks nearest faces, if nearest face normal points in same general direction then ignore
    std::vector<int> filterCells(SurfaceMesh* meshA, SurfaceMesh* meshB) const;

    ///
    /// \brief Creates a line mesh by connecting points of the given faces of meshA with with
    /// random points on random faces on meshB with strands
    /// \param meshA mesh on one side of the connective tissue
    /// \param faces list of faces on meshA that will have strands on them
    /// \param meshB mesh on the other side of the connective tissue
    /// \return the generate mesh
    std::shared_ptr<LineMesh> createStrands(
        SurfaceMesh*            meshA,
        const std::vector<int>& faces,
        SurfaceMesh*            meshB) const;

    /// \return A random point on the given face and the given mesh
    const Vec3d generateRandomPointOnFace(SurfaceMesh* mesh, int face) const;

    int    m_segmentsPerStrand = 3;
    double m_strandsPerFace    = 1;
};
} // namespace imstk