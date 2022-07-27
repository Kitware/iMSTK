/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometryMap.h"
#include "imstkGeometry.h"

namespace imstk
{
GeometryMap::GeometryMap()
{
    // Set 2 ports for each, inputs will be set to outputs
    setNumInputPorts(2);

    // Both inputs required
    setRequiredInputType<Geometry>(0);
    setRequiredInputType<Geometry>(1);

    // Set 1 output port, the child
    setNumOutputPorts(1);
}
} // namespace imstk