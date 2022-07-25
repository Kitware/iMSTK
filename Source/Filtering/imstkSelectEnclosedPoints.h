/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkDataArray.h"

namespace imstk
{
class PointSet;
class SurfaceMesh;

///
/// \class CleanMesh
///
/// \brief This filter will select the points within a surfacemesh. It can
/// either prune or return the mask
///
class SelectEnclosedPoints : public GeometryAlgorithm
{
public:
    SelectEnclosedPoints();
    ~SelectEnclosedPoints() override = default;

public:
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);
    void setInputPoints(std::shared_ptr<PointSet> inputPoints);
    std::shared_ptr<PointSet> getOutputPoints() const;

    imstkGetMacro(UsePruning, bool);
    imstkGetMacro(Tolerance, double);
    imstkGetMacro(InsideOut, bool);
    imstkGetMacro(IsInsideMask, std::shared_ptr<DataArray<unsigned char>>);

    imstkSetMacro(UsePruning, bool);
    imstkSetMacro(Tolerance, double);
    imstkSetMacro(InsideOut, bool);

protected:
    void requestUpdate() override;

private:
    bool   m_UsePruning = true;
    double m_Tolerance  = 0.0;
    bool   m_InsideOut  = false;
    std::shared_ptr<DataArray<unsigned char>> m_IsInsideMask = nullptr;
};
} // namespace imstk