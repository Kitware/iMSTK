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
}