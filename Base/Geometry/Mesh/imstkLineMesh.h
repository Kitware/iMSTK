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

#ifndef imstkLineMesh_h
#define imstkLineMesh_h

#include <memory>

#include "imstkMesh.h"


namespace imstk
{

///
/// \class LineMesh
///
/// \brief Base class for all volume mesh types
///
class LineMesh : public Mesh
{
public:

    ///
    /// \brief Constructor
    ///
    LineMesh() : Mesh(Geometry::Type::LineMesh) {}

    ///
    /// \brief Default destructor
    ///
    ~LineMesh() = default;

    ///
    /// \brief
    ///
    virtual void clear();

    ///
    /// \brief
    ///
    virtual void print() const override;

    ///
    /// \brief
    ///
    virtual double getVolume() const;

    ///
    /// \brief
    ///
    void setConnectivity(const std::vector<std::vector<int> >& lines);

    ///
    /// \brief
    ///
    int getNumLines();

    ///
    /// \brief
    ///
    std::vector<std::vector<int> > getLines() const;

    ///
    /// \brief
    ///
    std::vector<int> getLine(int index) const;

private:
    std::vector<std::vector<int> > m_lines; ///> line connectivity
};

} // imstk

#endif // ifndef imstkLineMesh_h
