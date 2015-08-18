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

#include "IO/IOMesh.h"

#include "Core/BaseMesh.h"
#include "Core/Factory.h"
#include "Core/MakeUnique.h"
#include "IO/IOMeshDelegate.h"

///
/// \brief DelegatorType. Holds a list of factory delegates that take care the
/// actual read operation. This is the dispatcher of delegators for readers
/// and writers for mesh formats.
class IOMesh::DelegatorType
{
    typedef std::function<std::shared_ptr<IOMeshDelegate>()> DelegatorFunction;

public:
    DelegatorType(IOMesh *ioMesh) : io(ioMesh){}
    void addDefaultDelegator(const MeshFileType &type, const std::string &delegateName)
    {
        auto f = [=]()
        {
            auto delegate = Factory<IOMeshDelegate>::createDefault(delegateName);
            if(delegate)
            {
                delegate->setSource(io);
            }
            return delegate;
        };
        this->delegatorList.emplace(type,f);
    }
    void addGroupDelegator(const MeshFileType &type, const std::string &delegateName, const int &group)
    {
        auto f = [=]()
        {
            auto delegate = Factory<IOMeshDelegate>::createSubclassForGroup(delegateName,group);
            if(delegate)
            {
                delegate->setSource(io);
            }
            return delegate;
        };
        this->delegatorList.emplace(type,f);
    }
    std::shared_ptr<IOMeshDelegate> get(const MeshFileType &type)
    {
        return this->delegatorList.at(type)();
    }

public:
    std::map<MeshFileType,DelegatorFunction> delegatorList;
    IOMesh *io;
};

IOMesh::IOMesh(const IOMesh::ReaderGroup &priorityGroup) :
    fileName(""),
    fileType(MeshFileType::Unknown),
    mesh(nullptr),
    delegator(make_unique<DelegatorType>(this))
{
    //
    // VTK io for some vtk files (use only vtk to read these files)
    this->delegator->addDefaultDelegator(MeshFileType::VTK,"IOMeshDelegate");
    this->delegator->addDefaultDelegator(MeshFileType::VTU,"IOMeshDelegate");
    this->delegator->addDefaultDelegator(MeshFileType::VTP,"IOMeshDelegate");
    //
    // Set the vega io, only vega can read/write those files
    this->delegator->addDefaultDelegator(MeshFileType::VEG,"IOMeshVegaDelegate");
    //
    // The readers for obj,stl and ply are based on a priority group (defaults to vtk io's)
    this->delegator->addGroupDelegator(MeshFileType::OBJ,"IOMeshDelegate",priorityGroup);
    this->delegator->addGroupDelegator(MeshFileType::STL,"IOMeshDelegate",priorityGroup);
    this->delegator->addGroupDelegator(MeshFileType::PLY,"IOMeshDelegate",priorityGroup);
    //
    // Default reader for 3ds filetypes is assimp
    this->delegator->addGroupDelegator(MeshFileType::ThreeDS,"IOMeshDelegate",ReaderGroup::Assimp);
    //
    // Default reader for unknown filetypes
    this->delegator->addGroupDelegator(MeshFileType::Unknown,"IOMeshDelegate",priorityGroup);
}

IOMesh::~IOMesh() {}
void IOMesh::read(const std::string& filePath)
{
    this->fileName = filePath;
    this->fileType = this->getFileExtension();
    auto reader = this->delegator->get(this->fileType);
    if(reader)
    {
        reader->read();
    }
    else
    {
        std::cerr << "Error: No reader found for " << this->fileName << std::endl;
    }

}
void IOMesh::write(const std::string& /*filePath*/)
{
}
IOMesh::MeshFileType IOMesh::getFileExtension() const
{
    auto extensionType = MeshFileType::Unknown;
    if (this->fileName.length() == 0)
    {
        std::cerr << "IOMesh: Error invalid file name." << std::endl;
        return extensionType;
    }

    std::string extension = this->fileName.substr(this->fileName.find_last_of(".") + 1);

    if (extension =="vtk" || extension =="VTK")
    {
        extensionType = MeshFileType::VTK;
        return extensionType;
    }
    if (extension =="vtp" || extension =="VTP")
    {
        extensionType = MeshFileType::VTP;
        return extensionType;
    }
    if (extension =="vtu" || extension =="VTU")
    {
        extensionType = MeshFileType::VTU;
        return extensionType;
    }
    if (extension =="obj" || extension =="OBJ")
    {
        extensionType = MeshFileType::OBJ;
        return extensionType;
    }
    if (extension =="stl" || extension =="STL")
    {
        extensionType = MeshFileType::STL;
        return extensionType;
    }
    if (extension =="ply" || extension =="PLY")
    {
        extensionType = MeshFileType::PLY;
        return extensionType;
    }
    if (extension =="veg" || extension =="VEG")
    {
        extensionType = MeshFileType::VEG;
        return extensionType;
    }
    if (extension =="3ds" || extension =="3DS")
    {
        extensionType = MeshFileType::ThreeDS;
        return extensionType;
    }
    return extensionType;
}
std::shared_ptr<Core::BaseMesh> IOMesh::getMesh()
{
    return this->mesh;
}
void IOMesh::setMesh(std::shared_ptr< Core::BaseMesh > newMesh)
{
    this->mesh = newMesh;
}
const std::string& IOMesh::getFileName() const
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
