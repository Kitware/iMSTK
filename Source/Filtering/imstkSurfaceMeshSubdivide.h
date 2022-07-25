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
/// \class SubdivideSurfaceMesh
///
/// \brief This filter subidivdes the triangles of a SurfaceMesh into more
/// smaller triangles
/// \todo: test with LineMesh
///
class SurfaceMeshSubdivide : public GeometryAlgorithm
{
public:
    enum class Type
    {
        LINEAR,
        LOOP,
        BUTTERFLY
    };

public:
    SurfaceMeshSubdivide();
    ~SurfaceMeshSubdivide() override = default;

public:
    std::shared_ptr<SurfaceMesh> getOutputMesh();

    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

    imstkGetMacro(SubdivisionType, Type);
    imstkGetMacro(NumberOfSubdivisions, int);
    imstkSetMacro(SubdivisionType, Type);
    imstkSetMacro(NumberOfSubdivisions, int);

protected:
    void requestUpdate() override;

private:
    Type m_SubdivisionType      = Type::LINEAR;
    int  m_NumberOfSubdivisions = 1;
};
} // namespace imstk