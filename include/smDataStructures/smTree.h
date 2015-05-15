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

#ifndef SMTREE_H
#define SMTREE_H

// STL includes
#include <memory>

namespace smtk {

template<typename NodeType>
class smTree
{
public:
    smTree() : root(nullptr) {}
    ~smTree();

    void setRoot(const std::shared_ptr<NodeType> rootNode)
    {
        this->root = rootNode;
    }

    std::shared_ptr<NodeType> getRoot()
    {
        return this->root;
    }

private:
    std::shared_ptr<NodeType> root;
};

} // smtk

#endif // SMTREE_H
