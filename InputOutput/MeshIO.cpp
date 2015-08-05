// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "MeshIO.h"

MeshIO::MeshIO(): mesh(nullptr), fileName(""), fileType(MeshFileType::Unknown) {}
MeshIO::~MeshIO() {}
void MeshIO::read ( const std::string& filename )
{
    this->fileName = filename;
}
void MeshIO::write ( const std::string& filename )
{
}
void MeshIO::checkFileType( )
{
    if(this->fileName.length() == 0)
    {
        std::cerr << "MeshIO: Error invalid filename." << std::endl;
        return;
    }
    std::string extension = this->fileName.substr ( this->fileName.find_last_of ( "." ) + 1 );

    if ( extension == "vtk" )
    {
        this->fileType = MeshFileType::VTK;
    }
    if ( extension == "vtp" )
    {
        this->fileType = MeshFileType::VTP;
    }
    if ( extension == "obj" )
    {
        this->fileType = MeshFileType::OBJ;
    }
    if ( extension == "stl" )
    {
        this->fileType = MeshFileType::STL;
    }
    if ( extension == "ply" )
    {
        this->fileType = MeshFileType::PLY;
    }
    if ( extension == "veg" )
    {
        this->fileType = MeshFileType::VEG;
    }
    this->fileType = MeshFileType::Unknown;
}
std::shared_ptr< Core::BaseMesh > MeshIO::getMesh()
{
    return this->mesh;
}
void MeshIO::setMesh(std::shared_ptr< Core::BaseMesh > newMesh)
{
    this->mesh = newMesh;
}
const std::string& MeshIO::getFilename() const
{
    return this->fileName;
}
void MeshIO::setFilename(const std::string& filename)
{
    this->fileName = filename;
}
const MeshIO::MeshFileType &MeshIO::getFileType() const
{
    return fileType;
}
