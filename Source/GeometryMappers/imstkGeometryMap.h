/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
