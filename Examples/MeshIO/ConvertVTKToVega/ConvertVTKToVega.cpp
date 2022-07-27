/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
