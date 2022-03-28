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

#pragma once

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class Geometry;

///
/// \class GeometryMap
///
/// \brief Base class for any geometric map
///
class GeometryMap : public GeometryAlgorithm
{
protected:
    GeometryMap();

public:
    virtual ~GeometryMap() = default;

    ///
    /// \brief Returns the string class name
    ///
    /// Required for using IMSTK_TYPE_MACRO
    virtual const std::string getTypeName() const = 0;

    ///
    /// \brief Compute the map, usually called once on startup
    /// 
    virtual void compute() = 0;

    ///
    /// \brief Get/Set parent geometry
    ///@{
    void setParentGeometry(std::shared_ptr<Geometry> parent) { setInput(parent, 0); }
    std::shared_ptr<Geometry> getParentGeometry() const { return getInput(0); }
    ///@}

    ///
    /// \brief Get/Set child geometry
    ///@{
    void setChildGeometry(std::shared_ptr<Geometry> child) { setInput(child, 1); }
    std::shared_ptr<Geometry> getChildGeometry() const { return getInput(1); }
    ///@}
};
} // namespace imstk
