/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCellMesh.h"
#include "imstkLogger.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPointSet.h"

namespace imstk
{
std::shared_ptr<PbdModel>
PbdObject::getPbdModel()
{
    m_pbdModel = std::dynamic_pointer_cast<PbdModel>(m_dynamicalModel);
    return m_pbdModel;
}

void
PbdObject::setBodyFromGeometry()
{
    std::shared_ptr<PbdBody> body = getPbdBody();
    if (body->bodyType == PbdBody::Type::RIGID)
    {
        setRigidBody(*body);
    }
    else
    {
        if (m_physicsGeometry != nullptr)
        {
            auto pointSet = std::dynamic_pointer_cast<PointSet>(m_physicsGeometry);
            CHECK(pointSet != nullptr) << "PbdObject " << m_name << " only supports PointSet geometries";
            setDeformBodyFromGeometry(*body, pointSet);
        }
    }

    // Set the geometry to all the functors that need it
    // Not a good solution
    auto functors = m_pbdModel->getConfig()->getFunctors();
    for (auto& functorArray : functors)
    {
        for (auto& functor : functorArray.second)
        {
            if (auto bodyFunctor = std::dynamic_pointer_cast<PbdBodyConstraintFunctor>(functor))
            {
                if (bodyFunctor->m_bodyIndex == body->bodyHandle)
                {
                    auto pointSet = std::dynamic_pointer_cast<PointSet>(m_physicsGeometry);
                    CHECK(pointSet != nullptr) <<
                        "Tried to generate constraints with functor on PbdObject " << m_name << " but "
                        " object does not have PointSet geometry";
                    bodyFunctor->setGeometry(pointSet);
                }
            }
        }
    }
}

bool
PbdObject::initialize()
{
    m_pbdModel = std::dynamic_pointer_cast<PbdModel>(m_dynamicalModel);
    if (m_pbdModel == nullptr)
    {
        LOG(FATAL) << "PbdObject " << m_name << " was not given a PbdModel. Please PbdObject::setDynamicalModel";
        return false;
    }

    setBodyFromGeometry();

    // Set up maps before updating geometry
    DynamicObject::initialize();

    updateGeometries();

    return true;
}

void
PbdObject::setDynamicalModel(std::shared_ptr<AbstractDynamicalModel> dynaModel)
{
    // todo: If already has another model, should remove the corresponding body?
    m_pbdModel       = std::dynamic_pointer_cast<PbdModel>(dynaModel);
    m_dynamicalModel = dynaModel;

    // If the model already has a pbd body for this PbdObject remove it from
    // that prior model
    if (m_pbdBody != nullptr)
    {
        CHECK(m_pbdModel != nullptr) <<
            "PbdObject has a PbdBody but cannot find associated PbdModel?";
        m_pbdModel->removeBody(m_pbdBody);
    }
    m_pbdBody = m_pbdModel->addBody();
}

void
PbdObject::updatePhysicsGeometry()
{
    CHECK(m_physicsGeometry != nullptr) << "DynamicObject \"" << m_name
                                        << "\" expects a physics geometry, none was provided";

    DynamicObject::updatePhysicsGeometry();

    if (m_pbdBody->bodyType == PbdBody::Type::RIGID)
    {
        // Pbd has a special case here for rigid body as it cannot apply the
        // Apply the transform back to the geometry
        // If called before body is init'd use initial pose
        if (m_pbdBody->vertices->size() > 0)
        {
            m_physicsGeometry->setTranslation((*m_pbdBody->vertices)[0]);
            m_physicsGeometry->setRotation((*m_pbdBody->orientations)[0]);
        }
        m_physicsGeometry->updatePostTransformData();
    }
}

///
/// \brief Convience function. Set the bodyArrPtr to the specified attribute
/// or allocate it with initValue set
///
template<typename T>
static void
setOrAllocate(std::shared_ptr<T>& bodyArrPtr,
              std::shared_ptr<PointSet> pointSet, const std::string& attributeName,
              typename T::ValueType initValue)
{
    // If the input mesh has the attribute already use those
    std::shared_ptr<AbstractDataArray> velocities = pointSet->getVertexAttribute(attributeName);
    if (velocities != nullptr
        && velocities->getNumberOfComponents() == T::NumComponents
        && velocities->getScalarType() == IMSTK_DOUBLE
        && std::dynamic_pointer_cast<T>(velocities)->size() == pointSet->getNumVertices())
    {
        bodyArrPtr = std::dynamic_pointer_cast<T>(velocities);
    }
    // If not, put existing (0 initialized velocities) on mesh
    else
    {
        bodyArrPtr = std::make_shared<T>(pointSet->getNumVertices());
        bodyArrPtr->fill(initValue);
        pointSet->setVertexAttribute(attributeName, bodyArrPtr);
    }
}

void
PbdObject::setDeformBodyFromGeometry(PbdBody& body, std::shared_ptr<PointSet> geom)
{
    body.vertices     = geom->getVertexPositions();
    body.prevVertices = std::make_shared<VecDataArray<double, 3>>(*body.vertices);

    const int numParticles = body.vertices->size();

    // Initialize Mass+InvMass
    {
        // If the input mesh has per vertex masses, use those
        std::shared_ptr<AbstractDataArray> masses = geom->getVertexAttribute("Mass");
        if (masses != nullptr && masses->getNumberOfComponents() == 1
            && masses->getScalarType() == IMSTK_DOUBLE && masses->size() == numParticles)
        {
            body.masses = std::dynamic_pointer_cast<DataArray<double>>(masses);
            body.invMasses->resize(body.masses->size());
            for (int i = 0; i < body.masses->size(); i++)
            {
                (*body.invMasses)[i] = ((*body.masses)[i] == 0.0) ? 0.0 : 1.0 / (*body.masses)[i];
            }
        }
        // If not, initialize as uniform and put on mesh
        else
        {
            // Initialize as uniform
            body.masses    = std::make_shared<DataArray<double>>(numParticles);
            body.invMasses = std::make_shared<DataArray<double>>(numParticles);

            body.masses->fill(body.uniformMassValue);
            body.invMasses->fill((body.uniformMassValue != 0.0) ?
                1.0 / body.uniformMassValue : 0.0);

            geom->setVertexAttribute("Mass", body.masses);
        }
        geom->setVertexAttribute("InvMass", body.invMasses);
    }

    setOrAllocate(body.velocities, geom, "Velocities", Vec3d::Zero());

    if (body.getOriented())
    {
        // Initialize Inertia + Inv tensors
        {
            // Initialize as uniform
            body.inertias    = std::make_shared<StdVectorOfMat3d>(numParticles);
            body.invInertias = std::make_shared<StdVectorOfMat3d>(numParticles);

            std::fill(body.inertias->begin(), body.inertias->end(), Mat3d::Identity());
            std::fill(body.invInertias->begin(), body.invInertias->end(), Mat3d::Identity());
        }

        // Initialize orientations
        // Expects Orientation Data to be Quaternions in wxyz order,
        // initialize with Identity otherwise
        {
            auto orientations = std::dynamic_pointer_cast<VecDataArray<double, 4>>(geom->getVertexAttribute("Orientations"));
            if (orientations != nullptr && orientations->size() == numParticles)
            {
                body.orientations = std::make_shared<StdVectorOfQuatd>(numParticles);
                for (int i = 0; i < orientations->size(); i++)
                {
                    (*body.orientations)[i] = Quatd((*orientations)[i][0], (*orientations)[i][1],
                        (*orientations)[i][2], (*orientations)[i][3]);
                }
            }
            else
            {
                body.orientations = std::make_shared<StdVectorOfQuatd>(numParticles, Quatd::Identity());
            }
        }
        body.prevOrientations = std::make_shared<StdVectorOfQuatd>(*body.orientations);

        setOrAllocate(body.angularVelocities, geom, "AngularVelocities", Vec3d::Zero());
    }

    // Overwrite some masses for specified fixed points
    body.fixedNodeInvMass = std::unordered_map<int, double>();
    for (auto i : body.fixedNodeIds)
    {
        DataArray<double>& invMasses = *body.invMasses;
        CHECK(i < numParticles && i >= 0) << "Tried to fix particle " << i
                                          << " but there only exist " << numParticles << " particles";
        body.fixedNodeInvMass[i] = invMasses[i];
        invMasses[i] = 0.0;
    }
}

///
/// \brief If array already exists does ensure size 1, If not allocates an array with value val
///
template<typename ArrPtrType, typename ValueType>
static void
setOrAllocateRigid(ArrPtrType& array, const ValueType val)
{
    using ArrType = typename ArrPtrType::element_type;
    // If array already exists, resize to 1, if no existing elements set val
    if (array != nullptr)
    {
        array->resize(1);
        if (array->size() == 0)
        {
            *array = { val };
        }
    }
    // If array doesn't exist create it with value val
    else
    {
        array  = std::make_shared<ArrType>(1);
        *array = { val };
    }
}

void
PbdObject::setRigidBody(PbdBody& body)
{
    // Basically a PbdBody with a single particle
    setOrAllocateRigid(body.vertices, Vec3d::Zero());
    body.prevVertices = std::make_shared<VecDataArray<double, 3>>(*body.vertices);

    // Initialize Mass+InvMass
    body.masses     = std::make_shared<DataArray<double>>();
    body.invMasses  = std::make_shared<DataArray<double>>();
    *body.masses    = { body.uniformMassValue };
    *body.invMasses = { (body.uniformMassValue != 0.0) ? 1.0 / body.uniformMassValue : 0.0 };

    setOrAllocateRigid(body.velocities, Vec3d::Zero());
    setOrAllocateRigid(body.inertias, Mat3d::Identity());
    body.invInertias  = std::make_shared<StdVectorOfMat3d>();
    *body.invInertias = { (*body.inertias)[0].inverse() };

    setOrAllocateRigid(body.orientations, Quatd::Identity());
    body.prevOrientations = std::make_shared<StdVectorOfQuatd>(*body.orientations);
    setOrAllocateRigid(body.angularVelocities, Vec3d::Zero());

    // Overwrite some masses for specified fixed points
    body.fixedNodeInvMass = std::unordered_map<int, double>();
}

void
PbdObject::computeCellConstraintMap()
{
    // Note: The PBD Object and constraints must be initialized before calling this function
    this->initialize();

    CHECK(m_physicsGeometry != nullptr) << "PbdObject \"" << m_name
                                        << "\" requires physics geometry to compute CellConstraint map";

    // If the map already exists, clear it and recalculate
    if (m_pbdBody->cellConstraintMap.empty() == false)
    {
        m_pbdBody->cellConstraintMap.clear();
        LOG(INFO) << "PbdObject \"" << m_name
                  << "\" already has a CellConstraintMap. Cleared and recalculated \n";
    }

    // Get body id
    int bodyId = m_pbdBody->bodyHandle;

    // Mesh data
    auto      cellMesh     = std::dynamic_pointer_cast<AbstractCellMesh>(this->getPhysicsGeometry());
    auto      cellVerts    = std::dynamic_pointer_cast<DataArray<int>>(cellMesh->getAbstractCells()); // underlying 1D array
    const int vertsPerCell = cellMesh->getAbstractCells()->getNumberOfComponents();

    // Constraint Data for all currently existing constraints
    std::shared_ptr<PbdConstraintContainer> constraintsPtr = this->getPbdModel()->getConstraints();
    CHECK(constraintsPtr != nullptr) << "PbdObject \"" << m_name
                                     << "\" does not have constraints in computeCellConstraintMap";

    const std::vector<std::shared_ptr<PbdConstraint>>& constraints = constraintsPtr->getConstraints();

    //For each cell, find all associated constraints
    std::vector<int> cellVertIds(vertsPerCell);
    for (int cellId = 0; cellId < cellMesh->getNumCells(); cellId++)
    {
        // Get all the vertex ids for this cell
        // The whole range gets overwritten for each iteration
        for (int vertId = 0; vertId < vertsPerCell; vertId++)
        {
            cellVertIds[vertId] = (*cellVerts)[cellId * vertsPerCell + vertId];
        }

        // Search all constraints for those that involve the same vertices as the cell
        for (auto& constraint : constraints)
        {
            const std::vector<PbdParticleId>& cVertexIds = constraint->getParticles(); ///< Vertices that are part of the constraint
            std::unordered_set<int>           constraintVertIds;

            // Check that constraint involves this body and get associated vertices
            bool isBody = false;
            for (int cVertId = 0; cVertId < cVertexIds.size(); cVertId++)
            {
                if (cVertexIds[cVertId].first == bodyId)
                {
                    constraintVertIds.insert(cVertexIds[cVertId].second);
                    isBody = true;
                }
            }
            if (isBody == false)
            {
                continue;
            }

            // Check if cell vertices exists in the constraint
            for (int cellVertId = 0; cellVertId < vertsPerCell; cellVertId++)
            {
                std::unordered_set<int>::const_iterator indx = constraintVertIds.find(cellVertIds[cellVertId]);

                if (indx != constraintVertIds.end())
                {
                    // Make sure constraint has not already been added
                    bool exists = false;
                    for (int j = 0; j < m_pbdBody->cellConstraintMap[cellId].size(); j++)
                    {
                        if (constraint == m_pbdBody->cellConstraintMap[cellId][j])
                        {
                            exists = true;
                        }
                    }
                    if (exists == false)
                    {
                        std::shared_ptr<PbdConstraint> cpy = constraint;
                        m_pbdBody->cellConstraintMap[cellId].push_back(std::move(cpy));
                    }
                }
            }
        }
    }
}
} // namespace imstk