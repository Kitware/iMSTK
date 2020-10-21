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
#include "imstkAssimpMeshIO.h"
#include "imstkLogger.h"
#include "imstkMSHMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVegaMeshIO.h"
#include "imstkVTKMeshIO.h"

#include <sys/stat.h>

namespace imstk
{
std::shared_ptr<PointSet>
MeshIO::read(const std::string& filePath)
{
    bool isDir;

    CHECK(MeshIO::fileExists(filePath, isDir)) << "MeshIO::read error: file not found: " << filePath;

    if (isDir)
    {
        // Assume that the directory is a collection of DICOM files
        return VTKMeshIO::read(filePath, MeshFileType::DCM);
    }

    MeshFileType meshType = MeshIO::getFileType(filePath);
    switch (meshType)
    {
    case MeshFileType::VTK:
    case MeshFileType::VTU:
    case MeshFileType::VTP:
    case MeshFileType::STL:
    case MeshFileType::PLY:
    case MeshFileType::NRRD:
    case MeshFileType::NII:
    case MeshFileType::DCM:
        return VTKMeshIO::read(filePath, meshType);
        break;
    case MeshFileType::OBJ:
    case MeshFileType::DAE:
    case MeshFileType::FBX:
    case MeshFileType::_3DS:
        return AssimpMeshIO::read(filePath, meshType);
        break;
    case MeshFileType::VEG:
        return VegaMeshIO::read(filePath, meshType);
        break;
    case MeshFileType::MSH:
        return MSHMeshIO::read(filePath, meshType);
        break;
    case UNKNOWN:
    default:
        break;
    }

    LOG(FATAL) << "MeshIO::read error: file type not supported";
    return nullptr;
}

bool
MeshIO::fileExists(const std::string& file, bool& isDirectory)
{
    struct stat buf;
    if (stat(file.c_str(), &buf) == 0)
    {
        if (buf.st_mode & S_IFDIR)
        {
            isDirectory = true;
        }
        else
        {
            isDirectory = false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

const MeshFileType
MeshIO::getFileType(const std::string& filePath)
{
    MeshFileType meshType = MeshFileType::UNKNOWN;

    std::string extString = filePath.substr(filePath.find_last_of(".") + 1);

    CHECK(!extString.empty()) << "MeshIO::getFileType error: invalid file name";

    if (extString == "vtk" || extString == "VTK")
    {
        meshType = MeshFileType::VTK;
    }
    else if (extString == "vtp" || extString == "VTP")
    {
        meshType = MeshFileType::VTP;
    }
    else if (extString == "vtu" || extString == "VTU")
    {
        meshType = MeshFileType::VTU;
    }
    else if (extString == "obj" || extString == "OBJ")
    {
        meshType = MeshFileType::OBJ;
    }
    else if (extString == "stl" || extString == "STL")
    {
        meshType = MeshFileType::STL;
    }
    else if (extString == "ply" || extString == "PLY")
    {
        meshType = MeshFileType::PLY;
    }
    else if (extString == "dae" || extString == "DAE")
    {
        meshType = MeshFileType::DAE;
    }
    else if (extString == "fbx" || extString == "FBX")
    {
        meshType = MeshFileType::FBX;
    }
    else if (extString == "3ds" || extString == "3DS")
    {
        meshType = MeshFileType::_3DS;
    }
    else if (extString == "veg" || extString == "VEG")
    {
        meshType = MeshFileType::VEG;
    }
    else if (extString == "msh" || extString == "MSH")
    {
        meshType = MeshFileType::MSH;
    }
    else if (extString == "dcm" || extString == "DCM")
    {
        meshType = MeshFileType::DCM;
    }
    else if (extString == "nrrd" || extString == "NRRD")
    {
        meshType = MeshFileType::NRRD;
    }
    else if (extString == "nii" || extString == "NII")
    {
        meshType = MeshFileType::NII;
    }
    else
    {
        LOG(FATAL) << "MeshIO::getFileType error: unknown file extension";
    }

    return meshType;
}

bool
MeshIO::write(const std::shared_ptr<imstk::PointSet> imstkMesh, const std::string& filePath)
{
    MeshFileType meshType = MeshIO::getFileType(filePath);
    switch (meshType)
    {
    case MeshFileType::VEG:
        return VegaMeshIO::write(imstkMesh, filePath, meshType);
        break;
    case MeshFileType::NII:
    case MeshFileType::NRRD:
    case MeshFileType::VTU:
    case MeshFileType::VTK:
    case MeshFileType::VTP:
    case MeshFileType::STL:
    case MeshFileType::PLY:
        return VTKMeshIO::write(imstkMesh, filePath, meshType);
        break;
    }

    LOG(FATAL) << "MeshIO::write error: file type not supported";
    return false;
}
} // imstk
