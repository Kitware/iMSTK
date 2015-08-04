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
MeshIO::MeshIO() : mesh ( nullptr ) {}
void MeshIO::read ( const std::string& filename )
{
}
void MeshIO::write ( const std::string& filename )
{
}
MeshIO::MeshFileType MeshIO::checkFileType ( const std::string& filename )
{
    std::string extension = filename.substr ( filename.find_last_of ( "." ) + 1 );
    if ( extension == "vtk" )
    {
        return MeshFileType::VTK;
    }
    if ( extension == "vtp" )
    {
        return MeshFileType::VTP;
    }
    if ( extension == "obj" )
    {
        return MeshFileType::OBJ;
    }
    if ( extension == "stl" )
    {
        return MeshFileType::STL;
    }
    if ( extension == "ply" )
    {
        return MeshFileType::PLY;
    }
    if ( extension == "veg" )
    {
        return MeshFileType::VEG;
    }
    return MeshFileType::Unknown;
}
std::shared_ptr< Core::BaseMesh > MeshIO::getMesh()
{
    return this->mesh;
}
void MeshIO::setReaderDelegate ( std::shared_ptr< ReaderDelegate > delegate )
{
    this->readerDelegate = delegate;
    if ( this->readerDelegate )
    {
        this->readerDelegate->setReader ( this );
    }
}
