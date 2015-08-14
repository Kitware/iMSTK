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
#include "IO/IOMeshDelegate.h"
#include "Core/MakeUnique.h"

class IOMesh::DelegatorType
{
    typedef std::function<std::shared_ptr<IOMeshDelegate>()> DelegatorFunction;

public:
    DelegatorType(IOMesh *ioMesh) : io(ioMesh){}
    void addDefaultDelegator(const MeshFileType &type, const std::string &delegateName)
    {
        this->delegatorList[type] = [&]()
        {
            auto delegate = Factory<IOMeshDelegate>::createDefault(delegateName);
            if(delegate)
            {
                delegate->setSource(io);
            }
            return delegate;
        };
    }
    void addGroupDelegator(const MeshFileType &type, const std::string &delegateName, const int &group)
    {
        this->delegatorList[type] = [&]()
        {
            auto delegate = Factory<IOMeshDelegate>::createSubclassForGroup(delegateName,group);
            if(delegate)
            {
                delegate->setSource(io);
            }
            return delegate;
        };
    }
    std::shared_ptr<IOMeshDelegate> get(const MeshFileType &type)
    {
        auto f = static_cast<DelegatorFunction>(this->delegatorList.at(type));
        auto delegate = Factory<IOMeshDelegate>::createSubclassForGroupAs("VegaMeshDelegate",ReaderGroup::Vega);
        auto fun = f();
        return f();
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
    this->delegator->addDefaultDelegator(MeshFileType::VTK,"VTKMeshReaderDelegate");
    this->delegator->addDefaultDelegator(MeshFileType::VTU,"VTKMeshReaderDelegate");
    this->delegator->addDefaultDelegator(MeshFileType::VTP,"VTKMeshReaderDelegate");
    //
    // Set the vega io, only vega can read/write those files
    this->delegator->addDefaultDelegator(MeshFileType::VEG,"VegaMeshDelegate");
    //
    // The readers for obj,stl and ply are based on a priority group (defaults to vtk io's)
    this->delegator->addGroupDelegator(MeshFileType::OBJ,"IOMeshDelegate",priorityGroup);
    this->delegator->addGroupDelegator(MeshFileType::STL,"IOMeshDelegate",priorityGroup);
    this->delegator->addGroupDelegator(MeshFileType::PLY,"IOMeshDelegate",priorityGroup);
    //
    // Default reader for unknown filetypes is assimp
    this->delegator->addDefaultDelegator(MeshFileType::VTP,"IOMeshDelegate");
    //
    // Default reader for 3ds filetypes is assimp
    this->delegator->addDefaultDelegator(MeshFileType::ThreeDS,"IOMeshDelegate");


}
IOMesh::~IOMesh() {}
void IOMesh::read(const std::string& filePath)
{
    this->fileName = filePath;
    this->checkFileType();
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
void IOMesh::checkFileType()
{
    if (this->fileName.length() == 0)
    {
        std::cerr << "IOMesh: Error invalid file name." << std::endl;
        return;
    }
    std::string extension = this->fileName.substr(this->fileName.find_last_of(".") + 1);

    if (extension =="vtk" || extension =="VTK")
    {
        this->fileType = MeshFileType::VTK;
        return;
    }
    if (extension =="vtp" || extension =="VTP")
    {
        this->fileType = MeshFileType::VTP;
        return;
    }
    if (extension =="obj" || extension =="OBJ")
    {
        this->fileType = MeshFileType::OBJ;
        return;
    }
    if (extension =="stl" || extension =="STL")
    {
        this->fileType = MeshFileType::STL;
        return;
    }
    if (extension =="ply" || extension =="PLY")
    {
        this->fileType = MeshFileType::PLY;
        return;
    }
    if (extension =="veg" || extension =="VEG")
    {
        this->fileType = MeshFileType::VEG;
        return;
    }
    if (extension =="3ds" || extension =="3DS")
    {
        this->fileType = MeshFileType::ThreeDS;
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
