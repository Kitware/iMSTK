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

#include "imstkMeshIO.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVegaMeshIO.h"
#include "imstkVTKMeshIO.h"

using namespace imstk;

const std::string defaultFileName = iMSTK_DATA_ROOT "textured_organs/heart_volume.vtk";

int
main(int argc, char** argv)
{
    std::string vtkFileName  = defaultFileName;
    std::string vegaFileName = "convertedMesh.veg";

    if (argc > 1)
    {
        vtkFileName = std::string(argv[1]);
    }

    auto tetMesh = MeshIO::read<TetrahedralMesh>(vtkFileName);
    tetMesh->rotate(Vec3d(1.0, 0.0, 0.0), -1.3, Geometry::TransformType::ApplyToData);
    bool converted = VegaMeshIO::write(tetMesh, vegaFileName, MeshFileType::VEG);
    VTKMeshIO::write(tetMesh, "convertedMesh.vtk", MeshFileType::VTK);

    if (converted)
    {
        return 0;
    }
    return 1;
}
