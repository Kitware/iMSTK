/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkVolumetricMesh.h"
#include "imstkVecDataArray.h"

#include <array>

namespace imstk
{
///
/// \class TetrahedralMesh
///
/// \brief Represents a set of tetrahedrons & vertices via an array of
/// Vec3d double vertices & Vec4i integer indices
///
/// To allow for flexible configuration of the tetrahedral mesh the "StrainParameter"
/// attribute can be used.
class TetrahedralMesh : public VolumetricMesh<4>
{
public:
    TetrahedralMesh() = default;
    ~TetrahedralMesh() override = default;

    static std::string StrainParameterName;

    IMSTK_TYPE_NAME(TetrahedralMesh)

    ///
    /// \brief This method extracts the conforming triangular mesh from the tetrahedral mesh
    ///
    std::shared_ptr<SurfaceMesh> extractSurfaceMesh() override;

    ///
    /// \brief compute the barycentric weights of a given point in 3D space for a given the tetrahedra
    ///
    Vec4d computeBarycentricWeights(const int tetId, const Vec3d& pos) const override;

    ///
    /// \brief Compute the bounding box of a given tetrahedron
    ///
    void computeTetrahedronBoundingBox(const size_t& tetId, Vec3d& min, Vec3d& max) const;

// Accessors
    ///
    /// \brief Get/set method for removed elements from the mesh
    ///@{
    void setTetrahedraAsRemoved(const unsigned int tetId) { m_removedMeshElems[tetId] = true; }
    const std::vector<bool>& getRemovedTetrahedra() const { return m_removedMeshElems; }
    ///@}

    ///
    /// \brief Compute and return the volume of the tetrahedral mesh
    ///
    double getVolume() override;

    int getNumTetrahedra() const { return getNumCells(); }
    void setTetrahedraIndices(std::shared_ptr<VecDataArray<int, 4>> indices) { setCells(indices); }
    std::shared_ptr<VecDataArray<int, 4>> getTetrahedraIndices() const { return getCells(); }

    ///
    /// \brief Polymorphic clone, hides the declaration in superclass
    /// return own type
    ///
    std::unique_ptr<TetrahedralMesh> clone()
    {
        return std::unique_ptr<TetrahedralMesh>(cloneImplementation());
    }

    ///
    /// \brief Get/Set the strain parameters for the tetrahedral mesh the strain parameters are
    /// expected to be a VecDataArray<double, 3> with the same number of elements as the number of
    /// tetrahedra in the mesh
    /// index 0: Model type from the enum TetrahedralMeshStrainModelType
    /// index 1: Youngs modulus
    /// index 2: Poisson ratio
    ///@{
    std::shared_ptr<VecDataArray<double, 3>> getStrainParameters();
    void setStrainParameters(std::shared_ptr<VecDataArray<double, 3>> strainParameters);
///@}

protected:
    std::vector<bool> m_removedMeshElems;

private:
    TetrahedralMesh* cloneImplementation() const;
};
} // namespace imstk
