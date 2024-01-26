/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCompoundCD.h"

#include "imstkCollisionData.h"
#include "imstkCollisionUtils.h"
#include "imstkCompoundGeometry.h"
#include "imstkCDObjectFactory.h"
#include "imstkLogger.h"

namespace imstk
{
CompoundCD::CompoundCD()
{
    setRequiredInputType<CompoundGeometry>(0);
    setRequiredInputType<Geometry>(1);
    m_colData       = nullptr;
    m_colVectorData = std::make_shared<std::vector<std::shared_ptr<CollisionData>>>();
    setGenerateCD(true, true);
}

void
CompoundCD::requestUpdate()
{
    // Could be parallelized ...
    std::for_each(m_cdAlgorithms.begin(), m_cdAlgorithms.end(), [](auto algo) { algo->update(); });
}

bool
CompoundCD::areInputsValid()
{
    // TODO check validation ...
    bool valid = CollisionDetectionAlgorithm::areInputsValid();

    CHECK(valid) << "Invalid or missing inputs in CompoundCD";

    int  otherIndex = 1;
    auto compound   = std::dynamic_pointer_cast<CompoundGeometry>(getInput(0));
    if (compound == nullptr)
    {
        compound   = std::dynamic_pointer_cast<CompoundGeometry>(getInput(1));
        otherIndex = 0;
    }
    auto other = getInput(otherIndex);

    // disregard additions of geometry during runtime
    if (m_cdAlgorithms.size() > 0)
    {
        return valid;
    }

    for (size_t i = 0; i < compound->count(); ++i)
    {
        auto geom = compound->get(i);
        auto type = CDObjectFactory::getCDType(*geom, *other);

        if (type.empty())
        {
            LOG(WARNING) << "CompoundCD could not find a CD Algorithm for " << geom->getTypeName() << " and " <<
                other->getTypeName() << " skipping.";
        }
        else
        {
            auto algorithm = CDObjectFactory::makeCollisionDetection(type);
            algorithm->setInput(geom, 0);
            algorithm->setInput(other, 1);
            CHECK(algorithm->getCollisionData() != nullptr) << "Can't stack a vector data algorithm inside of CompoundCD";
            m_colVectorData->push_back(algorithm->getCollisionData());
            m_cdAlgorithms.push_back(algorithm);
        }
    }

    return valid;
}
} // namespace imstk