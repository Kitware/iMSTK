#include "imstkAbstractDynamicalModel.h"

namespace imstk
{
bool
AbstractDynamicalModel::isGeometryValid(const std::shared_ptr<Geometry> geometry)
{
    if (geometry)
    {
        // If no valid geometries specified all geometries work
        if (m_validGeometryTypes.size() == 0)
            return true;

        // If it exists in the set then it is valid geometry
        if (m_validGeometryTypes.count(geometry->getType()))
            return true;
        else
        {
            LOG(WARNING) << "The geometry is not supported!!";
        }
    }
    else
    {
        LOG(WARNING) << "The geometry is not a valid pointer";
    }

    return false;
}

void
AbstractDynamicalModel::setModelGeometry(std::shared_ptr<Geometry> geometry)
{
    if (isGeometryValid(geometry))
        m_geometry = geometry;
    else
        LOG(WARNING) << "Invalid geometry for Model";
}

} // imstk