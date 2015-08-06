// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the"License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an"AS IS"BASIS,
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

#include"IOMesh.h"

IOMesh::IOMesh(IOMesh::ReaderGroup priorityGroup) : mesh(nullptr), fileName(""), fileType(MeshFileType::Unknown)
{
    delegatorList[MeshFileType::VTK] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createDefault("VTKMeshReaderDelegate");
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::VTU] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createDefault("VTKMeshReaderDelegate");
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::VTP] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createDefault("VTKMeshReaderDelegate");
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::VTP] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createDefault("VTKMeshReaderDelegate");
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::VEG] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createDefault("VegaMeshDelegate");
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::OBJ] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createSubclassForGroup("IODelegate",priorityGroup);
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::STL] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createSubclassForGroup("IODelegate",priorityGroup);
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::PLY] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createSubclassForGroup("IODelegate",priorityGroup);
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
    delegatorList[MeshFileType::Unknown] = [](std::shared_ptr<IOMesh> source)
    {
        auto delegate = Factory<IODelegate>::createSubclassForGroup("IODelegate",ReaderGroup::Assimp);
        if(delegate)
        {
            delegate->setSource(source);
        }
        return delegate;
    };
}
IOMesh::~IOMesh() {}
void IOMesh::read(const std::string& filePath)
{
    this->fileName = filePath;
    this->checkFileType();
    auto reader = this->delegatorList[this->fileType](this);
    if(reader)
    {
        reader->read();
    }
    else
    {
        std::cerr << "Error: Invalid reader!" << std::endl;
    }

}
void IOMesh::write(const std::string& filePath)
{
}
void IOMesh::checkFileType()
{
    if (this->fileName.length() == 0)
    {
        std::cerr << "IOMesh: Error invalid file name." << std::endl;
        return;
    }
    std::string extension = this->fileName.substr(this->fileName.find_last_of(".") + 1);

    if (extension =="vtk")
    {
        this->fileType = MeshFileType::VTK;
        return;
    }
    if (extension =="vtp")
    {
        this->fileType = MeshFileType::VTP;
        return;
    }
    if (extension =="obj")
    {
        this->fileType = MeshFileType::OBJ;
        return;
    }
    if (extension =="stl")
    {
        this->fileType = MeshFileType::STL;
        return;
    }
    if (extension =="ply")
    {
        this->fileType = MeshFileType::PLY;
        return;
    }
    if (extension =="veg")
    {
        this->fileType = MeshFileType::VEG;
        return;
    }
    this->fileType = MeshFileType::Unknown;
}
std::shared_ptr< Core::BaseMesh > IOMesh::getMesh()
{
    return this->mesh;
}
void IOMesh::setMesh(std::shared_ptr< Core::BaseMesh > newMesh)
{
    this->mesh = newMesh;
}
const std::string& IOMesh::getFilename() const
{
    return this->fileName;
}
void IOMesh::setFilename(const std::string& filePath)
{
    this->fileName = filePath;
}
const IOMesh::MeshFileType &IOMesh::getFileType() const
{
    return fileType;
}
