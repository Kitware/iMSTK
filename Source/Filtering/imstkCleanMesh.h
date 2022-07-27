/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class CleanMesh
///
/// \brief This filter can merge duplicate points and cells, it only works with
/// LineMesh and SurfaceMesh. It accepts a tolerance as a fraction of the length
/// of bounding box of the input data or an absolute tolerance
///
class CleanMesh : public GeometryAlgorithm
{
public:
    CleanMesh();
    ~CleanMesh() override = default;

    std::shared_ptr<SurfaceMesh> getOutputMesh() const;

    ///
    /// \brief Required input, port 0
    ///
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

    imstkGetMacro(UseAbsolute, bool);

    ///
    /// \brief Get/Set the tolerance for point merging, fraction of bounding box length
    ///@{
    void setTolerance(const double tolerance)
    {
        this->m_Tolerance = tolerance;
        m_UseAbsolute     = false;
    }

    imstkGetMacro(Tolerance, double);
    ///@}

    void setAbsoluteTolerance(const double tolerance)
    {
        this->m_AbsoluteTolerance = tolerance;
        m_UseAbsolute = true;
    }

    imstkGetMacro(AbsoluteTolerance, double);

protected:
    void requestUpdate() override;

private:
    double m_Tolerance = 0.0;
    double m_AbsoluteTolerance = 1.0;
    bool   m_UseAbsolute       = false;
};
} // namespace imstk