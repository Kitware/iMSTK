/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "EntityToVTK.h"
#include <math.h>

#include "vtksys/SystemTools.hxx"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkDoubleArray.h"
#include "vtkVertex.h"
#include "vtkXMLMultiBlockDataReader.h"
#include "vtkXMLMultiBlockDataWriter.h"
#include "vtkPolyDataWriter.h"
#include "vtkAppendPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkInformation.h"
#include "imstkDataTracker.h"

#include "imstkGeometryUtilities.h"
#include "imstkPbdModel.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdConstraint.h"
#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkObjectControllerGhost.h"
#include "imstkVisualModel.h"

namespace imstk
{
vtkSmartPointer<vtkMultiBlockDataSet>
EntityToVTK::convertToMultiBlock(std::shared_ptr<Entity> entity)
{
    vtkSmartPointer<vtkMultiBlockDataSet> mb = vtkSmartPointer<vtkMultiBlockDataSet>::New();
    unsigned int                          blockCount = 0;

    auto vertexInfo = this->getVertexInformation(entity);
    if (vertexInfo)
    {
        mb->SetBlock(blockCount, vertexInfo);
        mb->GetMetaData(blockCount)->Set(vtkMultiBlockDataSet::NAME(), "Vertex Info");
        blockCount++;
    }

    // auto physGeo = this->getPhysicalGeometry(entity);
    // if (physGeo)
    // {
    //     mb->SetBlock(blockCount, physGeo);
    //     mb->GetMetaData(blockCount)->Set(vtkMultiBlockDataSet::NAME(), "Physical Geometry");
    //     blockCount++;
    // }

    // auto visGeo = this->getVisualGeometry(entity);
    // if (visGeo)
    // {
    //     mb->SetBlock(blockCount, visGeo);
    //     mb->GetMetaData(blockCount)->Set(vtkMultiBlockDataSet::NAME(), "Visual Geometry");
    //     blockCount++;
    // }

    // auto collisionGeo = this->getCollisionGeometry(entity);
    // if (collisionGeo)
    // {
    //     mb->SetBlock(blockCount, collisionGeo);
    //     mb->GetMetaData(blockCount)->Set(vtkMultiBlockDataSet::NAME(), "Collision Geometry");
    //     blockCount++;
    // }

    // auto ghost = this->getGhost(entity);
    // if (ghost)
    // {
    //     mb->SetBlock(blockCount, ghost);
    //     mb->GetMetaData(blockCount)->Set(vtkMultiBlockDataSet::NAME(), "Ghost");
    //     blockCount++;
    // }

    if (blockCount == 0)
    {
        LOG(WARNING) << "warning:  " << entity->getName() << " entity type is not supported";
        return nullptr;
    }

    return mb;
}

vtkSmartPointer<vtkPolyData>
EntityToVTK::getVertexInformation(std::shared_ptr<Entity> entity)
{
    if (m_recordingType == RecordingType::NoRecording)
    {
        return nullptr;
    }

    auto pbdObject = std::dynamic_pointer_cast<PbdObject>(entity);
    if (pbdObject)
    {
        auto geometry = pbdObject->getPhysicsGeometry();
        if (geometry->isMesh())
        {
            auto pointSet = std::dynamic_pointer_cast<PointSet>(geometry);
            auto polydata = this->getVertexInformation(pointSet, pbdObject->getPbdBody(), pbdObject->getName());
            if (m_enableConstraintCounting)
            {
                this->addConstraintCount(pbdObject, polydata);
            }

            return polydata;
        }
        else
        {
            auto analyticalGeo = std::dynamic_pointer_cast<AnalyticalGeometry>(geometry);
            if (analyticalGeo)
            {
                auto polydata = this->getVertexInformation(analyticalGeo, pbdObject->getPbdBody(), pbdObject->getName());
                if (m_enableConstraintCounting)
                {
                    this->addConstraintCount(pbdObject, polydata);
                }
                return polydata;
            }
        }
    }

    LOG(DEBUG) << "warning: Vertex information is not supported for " << entity->getTypeName();
    return nullptr;
}

vtkSmartPointer<vtkDataObject>
EntityToVTK::getPhysicalGeometry(std::shared_ptr<Entity> entity)
{
    auto dynamicObject = std::dynamic_pointer_cast<DynamicObject>(entity);
    if (dynamicObject)
    {
        auto geometry = dynamicObject->getPhysicsGeometry();
        return this->geometryToData(geometry);
    }

    LOG(DEBUG) << "warning: Physical geometry is not supported for " << entity->getTypeName();
    return nullptr;
}

vtkSmartPointer<vtkDataObject>
EntityToVTK::getVisualGeometry(std::shared_ptr<Entity> entity)
{
    auto sceneObject = std::dynamic_pointer_cast<SceneObject>(entity);
    if (sceneObject)
    {
        auto geometry = sceneObject->getVisualGeometry();
        return this->geometryToData(geometry);
    }

    LOG(DEBUG) << "warning: Visual geometry is not supported for " << entity->getTypeName();
    return nullptr;
}

vtkSmartPointer<vtkDataObject>
EntityToVTK::getCollisionGeometry(std::shared_ptr<Entity> entity)
{
    auto sceneObject = std::dynamic_pointer_cast<DynamicObject>(entity);
    if (sceneObject)
    {
        auto geometry = sceneObject->getCollidingGeometry();
        return this->geometryToData(geometry);
    }

    LOG(DEBUG) << "warning: Collision geometry is not supported for " << entity->getTypeName();
    return nullptr;
}

vtkSmartPointer<vtkDataObject>
EntityToVTK::getGhost(std::shared_ptr<Entity> entity)
{
    auto ghost = entity->getComponent<ObjectControllerGhost>();
    if (ghost)
    {
        auto model = ghost->getGhostModel();
        return this->geometryToData(model->getGeometry());
    }

    LOG(DEBUG) << "warning: Ghost is not supported for " << entity->getTypeName();
    return nullptr;
}

vtkSmartPointer<vtkPolyData>
EntityToVTK::getVertexInformation(std::shared_ptr<PointSet> pointSet, std::shared_ptr<PbdBody> body, std::string name)
{
    vtkSmartPointer<vtkPolyData>  polydata = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints>    points   = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();

    vtkSmartPointer<vtkDoubleArray> velArray = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> displacementArray = vtkSmartPointer<vtkDoubleArray>::New();

    velArray->SetName("PBD Velocity");
    displacementArray->SetName("Displacement");

    velArray->SetNumberOfComponents(3);
    displacementArray->SetNumberOfComponents(3);

    int numVerticies = m_indexToRecord[name].size();
    velArray->SetNumberOfTuples(numVerticies * 3);
    displacementArray->SetNumberOfTuples(numVerticies * 3);
    int i = 0;
    bool foundNan = false;
    for (const auto& index : m_indexToRecord[name])
    {
        if (index >= pointSet->getNumVertices())
        {
            continue;
        }
        Vec3d currentPosition = pointSet->getVertexPosition(index);

        if (isnan(currentPosition.norm()))
        {
            foundNan = true;
        }

        vtkSmartPointer<vtkVertex> vertex = vtkSmartPointer<vtkVertex>::New();
        vertex->GetPointIds()->SetId(0, points->InsertNextPoint(currentPosition[0], currentPosition[1], currentPosition[2]));
        vertices->InsertNextCell(vertex);

        velArray->InsertTuple(i, (*body->velocities)[index].data());
        displacementArray->InsertTuple(i, (*body->prevVertices)[index].data());
        i++;
    }

    // if (foundNan)
    // {
    //     std::cout << "Found NaN Position" << std::endl;
    // }

    polydata->SetPoints(points);
    polydata->SetVerts(vertices);

    polydata->GetPointData()->AddArray(velArray);
    polydata->GetPointData()->AddArray(displacementArray);

    return polydata;
}

vtkSmartPointer<vtkPolyData>
EntityToVTK::getVertexInformation(std::shared_ptr<AnalyticalGeometry> analyticalGeo, std::shared_ptr<PbdBody> body, std::string name)
{
    vtkSmartPointer<vtkPolyData>  polydata = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints>    points   = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();

    vtkSmartPointer<vtkDoubleArray> velArray = vtkSmartPointer<vtkDoubleArray>::New();
    vtkSmartPointer<vtkDoubleArray> displacementArray = vtkSmartPointer<vtkDoubleArray>::New();

    velArray->SetName("PBD Velocity");
    displacementArray->SetName("Displacement");

    velArray->SetNumberOfComponents(3);
    displacementArray->SetNumberOfComponents(3);

    velArray->SetNumberOfTuples(3);
    displacementArray->SetNumberOfTuples(3);

    Vec3d currentPosition = analyticalGeo->getPosition();
    Vec3d displacement;

    vtkSmartPointer<vtkVertex> vertex = vtkSmartPointer<vtkVertex>::New();
    vertex->GetPointIds()->SetId(0, points->InsertNextPoint(currentPosition[0], currentPosition[1], currentPosition[2]));
    vertices->InsertNextCell(vertex);

    velArray->InsertTuple(0, (*body->velocities)[0].data());
    displacementArray->InsertTuple(0, (*body->prevVertices)[0].data());

    polydata->SetPoints(points);
    polydata->SetVerts(vertices);

    polydata->GetPointData()->AddArray(velArray);
    polydata->GetPointData()->AddArray(displacementArray);
    return polydata;
}

void
EntityToVTK::addConstraintCount(std::shared_ptr<PbdObject> pbdObject, vtkSmartPointer<vtkPolyData> polydata)
{
    vtkSmartPointer<vtkIntArray> constraintCountArray = vtkSmartPointer<vtkIntArray>::New();

    constraintCountArray->SetName("Constraint Count");
    auto constraints = pbdObject->getPbdModel()->getConstraints()->getConstraints();

    if (m_lastConstraintTotals.count(pbdObject->getName()) || m_lastConstraintTotals[pbdObject->getName()] != constraints.size())
    {
        m_lastConstraintTotals[pbdObject->getName()] = constraints.size();
        for (const auto& index : m_indexToRecord[pbdObject->getName()])
        {
            int count = 0;

            for (int j = 0; j < constraints.size(); j++)
            {
                auto particles = constraints[j]->getParticles();
                for (int k = 0; k < particles.size(); k++)
                {
                    if (particles[k].first == (*pbdObject->getPbdBody()).bodyHandle && particles[k].second == index)
                    {
                        count++;
                    }
                }
            }

            m_lastConstraintCounts[pbdObject->getName()][index] = count;
            constraintCountArray->InsertNextValue(count);
        }
    }
    else
    {
        for (const auto& index : m_indexToRecord[pbdObject->getName()])
        {
            constraintCountArray->InsertNextValue(m_lastConstraintCounts[pbdObject->getName()][index]);
        }
    }

    polydata->GetPointData()->AddArray(constraintCountArray);
}

vtkSmartPointer<vtkDataObject>
EntityToVTK::geometryToData(std::shared_ptr<Geometry> geometry)
{
    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    if (geometry)
    {
        if (geometry->isMesh())
        {
            auto lineMesh    = std::dynamic_pointer_cast<LineMesh>(geometry);
            auto surfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(geometry);
            auto tetMesh     = std::dynamic_pointer_cast<TetrahedralMesh>(geometry);
            auto hexMesh     = std::dynamic_pointer_cast<HexahedralMesh>(geometry);

            if (lineMesh)
            {
                polydata = GeometryUtils::copyToVtkPolyData(lineMesh);
            }
            else if (surfaceMesh)
            {
                polydata = GeometryUtils::copyToVtkPolyData(surfaceMesh);
            }
            else if (tetMesh)
            {
                return GeometryUtils::copyToVtkUnstructuredGrid(tetMesh);
            }
            else if (hexMesh)
            {
                return GeometryUtils::copyToVtkUnstructuredGrid(hexMesh);
            }
            else
            {
                LOG(WARNING) << "warning: Mesh type " << geometry->getTypeName() << " is not supported";
                return nullptr;
            }

            return polydata;
        }
        else
        {
            auto analyticalGeo = std::dynamic_pointer_cast<AnalyticalGeometry>(geometry);
            if (analyticalGeo)
            {
                auto surfaceMesh = GeometryUtils::toSurfaceMesh(analyticalGeo);
                polydata = GeometryUtils::copyToVtkPolyData(surfaceMesh);
                return polydata;
            }
        }
    }

    return nullptr;
}

void
EntityToVTK::addEntity(std::shared_ptr<Entity> entity)
{
    this->m_Entities.insert({ entity->getName(), entity });
}

bool
EntityToVTK::checkType(std::string name)
{
    if (strcmp(name.c_str(), "Vertex Info") == 0)
    {
        return this->m_writeVertexInfo;
    }
    else if (strcmp(name.c_str(), "Physical Geometry") == 0)
    {
        return this->m_writePhysicalGeo;
    }
    else if (strcmp(name.c_str(), "Visual Geometry") == 0)
    {
        return this->m_writeVisualGeo;
    }
    else if (strcmp(name.c_str(), "Collision Geometry") == 0)
    {
        return this->m_writeCollisionGeo;
    }
    else if (strcmp(name.c_str(), "Ghost") == 0)
    {
        return this->m_writeGhost;
    }

    return false;
}

void
EntityToVTK::writeObjectsToFile(std::string fileName)
{
    LOG(INFO) << "Writing all blocks to file " << fileName;
    vtkXMLMultiBlockDataWriter* writer = vtkXMLMultiBlockDataWriter::New();
    writer->SetFileName(fileName.c_str());
    writer->SetInputDataObject(this->m_outputBlock);
    writer->SetCompressionLevel(1);
    writer->Update();
    writer->Delete();
    LOG(INFO) << "Finished writing to file " << fileName;
}

void
EntityToVTK::writeObjectsToTimeSeries(vtkSmartPointer<vtkMultiBlockDataSet> mb, double timestep)
{
    if (!this->m_initializedTimeSeries)
    {
        if (!vtksys::SystemTools::PathExists(this->m_timeSeriesFilepath))
        {
            vtksys::SystemTools::RemoveADirectory(this->m_timeSeriesFilepath);
        }

        vtksys::SystemTools::MakeDirectory(this->m_timeSeriesFilepath);

        this->m_timeSeriesJson = "{\"file-series-version\" : \"1.0\",\n\"files\" : [\n";
        this->m_initializedTimeSeries = true;
    }

    vtkSmartPointer<vtkXMLMultiBlockDataWriter> writer      = vtkSmartPointer<vtkXMLMultiBlockDataWriter>::New();
    vtkSmartPointer<vtkMultiBlockDataSet>       outputBlock = vtkSmartPointer<vtkMultiBlockDataSet>::New();
    int                                         outputCount = 0;
    vtkSmartPointer<vtkMultiBlockDataSet>       temp;

    for (unsigned int i = 0; i < mb->GetNumberOfBlocks(); i++)
    {
        vtkSmartPointer<vtkMultiBlockDataSet> inner = vtkMultiBlockDataSet::SafeDownCast(mb->GetBlock(i));
        temp = vtkSmartPointer<vtkMultiBlockDataSet>::New();
        int innerCount = 0;

        for (unsigned int k = 0; k < inner->GetNumberOfBlocks(); k++)
        {
            std::string name = inner->GetMetaData((unsigned int) k)->Get(vtkMultiBlockDataSet::NAME());
            if (this->checkType(name))
            {
                temp->SetBlock(innerCount, inner->GetBlock(k));
                temp->GetMetaData((unsigned int) innerCount)->Set(vtkMultiBlockDataSet::NAME(), name);
                innerCount++;
            }
        }

        outputBlock->SetBlock(outputCount, temp);
        outputBlock->GetMetaData((unsigned int) outputCount)->Set(vtkMultiBlockDataSet::NAME(), mb->GetMetaData((unsigned int) i)->Get(vtkMultiBlockDataSet::NAME()));
        outputCount++;
    }

    std::string vtkName = "/Block_";
    vtkName  = this->m_timeSeriesFilepath + vtkName;
    vtkName += std::to_string(this->m_blockCount);
    vtkName += ".vtm";

    writer->SetFileName(vtkName.c_str());
    writer->SetInputDataObject(outputBlock);
    writer->Write();

    this->m_timeSeriesJson += "{ \"name\" : \"" + vtkName.substr(this->m_timeSeriesFilepath.size() + 1) + "\", \"time\" : " + std::to_string(timestep) + " },\n";
}

void
EntityToVTK::writeTimeSeriesJsonFile()
{
    std::string   fullPath = this->m_timeSeriesFilepath + "/blocks.vtm.series";
    std::ofstream file(fullPath);

    this->m_timeSeriesJson = this->m_timeSeriesJson.substr(0, this->m_timeSeriesJson.size() - 2);

    this->m_timeSeriesJson += "\n]}\n";
    file << this->m_timeSeriesJson;
    file.close();
    LOG(INFO) << "Writing to file " << this->m_timeSeriesFilepath;
}

void
EntityToVTK::recordObjectState(double timeStamp)
{
    if (this->m_timeSinceLastRecording == -1 || this->m_timeSinceLastRecording + timeStamp >= this->m_timeBetweenRecordings)
    {
        this->m_accumlatedTime += timeStamp;
        this->m_timesteps.push_back(this->m_accumlatedTime);
        vtkSmartPointer<vtkMultiBlockDataSet> objectsMb   = vtkSmartPointer<vtkMultiBlockDataSet>::New();
        // std::cout << this->m_blockCount << std::endl;
        unsigned int                          objectCount = 0;
        for (const auto& n : this->m_Entities)
        {
            if (m_indexToRecord[n.first].empty())
            {
                this->setupIndexToRecord(n.second);
            }
            vtkSmartPointer<vtkDataObject> output = this->convertToMultiBlock(n.second);
            if (output)
            {
                objectsMb->SetBlock(objectCount, output);
                objectsMb->GetMetaData(objectCount)->Set(vtkMultiBlockDataSet::NAME(), n.first);
                objectCount++;
            }
        }

        this->writeObjectsToTimeSeries(objectsMb, this->m_accumlatedTime);

        this->m_outputBlock->SetBlock(this->m_blockCount, objectsMb);
        this->m_outputBlock->GetMetaData(this->m_blockCount)->Set(vtkMultiBlockDataSet::NAME(), "Timestep " + std::to_string(this->m_accumlatedTime));
        this->m_blockCount++;
        this->m_timeSinceLastRecording = 0;
    }
    else
    {
        this->m_timeSinceLastRecording += timeStamp;
        this->m_accumlatedTime += timeStamp;
    }
}

void
EntityToVTK::setupIndexToRecord(std::shared_ptr<Entity> entity)
{
    auto pbdObject = std::dynamic_pointer_cast<PbdObject>(entity);
    if (pbdObject)
    {
        auto geometry = pbdObject->getPhysicsGeometry();
        if (geometry->isMesh())
        {
            auto pointSet = std::dynamic_pointer_cast<PointSet>(geometry);
            if (m_recordingType == RecordingType::VectorRecording)
            {
                this->findClosestPoints(pointSet, entity->getName());
            }
            else if (m_recordingType == RecordingType::BoundingBox)
            {
                this->findEnclosedPoints(pointSet, entity->getName());
            }
            else if (m_recordingType == RecordingType::SubsetRecording)
            {
                m_indexToRecord[entity->getName()] = m_subsetIds;
            }
            else
            {
                for (int i = 0; i < pointSet->getNumVertices(); i += m_recordingFrequency)
                {
                    m_indexToRecord[entity->getName()].push_back(i);
                }
            }
        }
        else
        {
            m_indexToRecord[entity->getName()].push_back(0);
        }
    }
}

void
EntityToVTK::findEnclosedPoints(std::shared_ptr<PointSet> pointSet, std::string name)
{
    for (int i = 0; i < pointSet->getNumVertices(); i++)
    {
        Vec3d vertex = pointSet->getVertexPosition(i);
        if ((m_lowerCornerBbox[0] <= vertex[0] && m_upperCornerBbox[0] >= vertex[0])
            && (m_lowerCornerBbox[1] <= vertex[1] && m_upperCornerBbox[1] >= vertex[1])
            && (m_lowerCornerBbox[2] <= vertex[2] && m_upperCornerBbox[2] >= vertex[2]))
        {
            m_indexToRecord[name].push_back(i);
        }
    }
}

void
EntityToVTK::findClosestPoints(std::shared_ptr<PointSet> pointSet, std::string name)
{
    for (const auto& vec : m_closestPoints)
    {
        double smallestDistance = (pointSet->getVertexPosition(0) - vec).norm();
        int    smallestIndex    = 0;
        for (int i = 1; i < pointSet->getNumVertices(); i++)
        {
            auto distance = (pointSet->getVertexPosition(i) - vec).norm();

            if (distance < smallestDistance)
            {
                smallestIndex    = i;
                smallestDistance = distance;
            }
        }

        m_indexToRecord[name].push_back(smallestIndex);
    }
}

void
EntityToVTK::compareMultiBlocks(std::string compareFilePath, std::string verificationFilePath, std::string objectName, std::string outputDir)
{
    vtkSmartPointer<vtkMultiBlockDataSet> compareMb      = this->readvtkMultiBlock(compareFilePath);
    vtkSmartPointer<vtkMultiBlockDataSet> verificationMb = this->readvtkMultiBlock(verificationFilePath);
    auto                                  dataTracker    = std::make_shared<DataTracker>();
    dataTracker->setFilename(outputDir);

    dataTracker->configureProbe("DisplacementRMS");
    dataTracker->configureProbe("AverageXDisplacement");
    dataTracker->configureProbe("AverageYDisplacement");
    dataTracker->configureProbe("AverageZDisplacement");
    dataTracker->configureProbe("MinDisplacement");
    dataTracker->configureProbe("MinDisplacementIndex");
    dataTracker->configureProbe("MaxDisplacement");
    dataTracker->configureProbe("MaxDisplacementIndex");

    dataTracker->configureProbe("VelocityRMS");
    dataTracker->configureProbe("AverageXVelocity");
    dataTracker->configureProbe("AverageYVelocity");
    dataTracker->configureProbe("AverageZVelocity");
    dataTracker->configureProbe("MinVelocity");
    dataTracker->configureProbe("MinVelocityIndex");
    dataTracker->configureProbe("MaxVelocity");
    dataTracker->configureProbe("MaxVelocityIndex");

    if (compareMb && verificationMb)
    {
        vtkSmartPointer<vtkMultiBlockDataSet> timestepCompareMb;
        vtkSmartPointer<vtkMultiBlockDataSet> innerCompareMb;
        vtkSmartPointer<vtkMultiBlockDataSet> timestepVerificationMb;
        vtkSmartPointer<vtkMultiBlockDataSet> innerVerificationMb;

        vtkSmartPointer<vtkPolyData> comparePolydata;
        vtkSmartPointer<vtkPolyData> verificationPolydata;

        vtkSmartPointer<vtkPoints> comparePoints;
        vtkSmartPointer<vtkPoints> verificationPoints;

        vtkSmartPointer<vtkPointData> comparePointData;
        vtkSmartPointer<vtkPointData> verificationPointData;

        for (unsigned int i = 0; i < compareMb->GetNumberOfBlocks(); i++)
        {
            timestepCompareMb      = vtkMultiBlockDataSet::SafeDownCast(compareMb->GetBlock(i));
            timestepVerificationMb = vtkMultiBlockDataSet::SafeDownCast(verificationMb->GetBlock(i));

            for (unsigned int j = 0; j < timestepCompareMb->GetNumberOfBlocks(); j++)
            {
                if (objectName.empty() || strcmp(timestepCompareMb->GetMetaData((unsigned int) j)->Get(vtkMultiBlockDataSet::NAME()), objectName.c_str()) == 0)
                {
                    innerCompareMb      = vtkMultiBlockDataSet::SafeDownCast(timestepCompareMb->GetBlock(j));
                    innerVerificationMb = vtkMultiBlockDataSet::SafeDownCast(timestepVerificationMb->GetBlock(j));

                    if (strcmp(innerCompareMb->GetMetaData((unsigned int) 0)->Get(vtkMultiBlockDataSet::NAME()), "Vertex Info") != 0
                        || strcmp(innerVerificationMb->GetMetaData((unsigned int) 0)->Get(vtkMultiBlockDataSet::NAME()), "Vertex Info") != 0)
                    {
                        LOG(WARNING) << "warning: Missing vertex information in multiblock. Stoping comparision";
                        return;
                    }

                    comparePolydata      = vtkPolyData::SafeDownCast(innerCompareMb->GetBlock(0));
                    verificationPolydata = vtkPolyData::SafeDownCast(innerVerificationMb->GetBlock(0));

                    comparePoints      = comparePolydata->GetPoints();
                    verificationPoints = verificationPolydata->GetPoints();

                    comparePointData      = comparePolydata->GetPointData();
                    verificationPointData = verificationPolydata->GetPointData();

                    auto compareVelocity      = vtkDataArray::SafeDownCast(comparePointData->GetAbstractArray("PBD Velocity"));
                    auto verificationVelocity = vtkDataArray::SafeDownCast(verificationPointData->GetAbstractArray("PBD Velocity"));

                    double displacementRMS   = 0;
                    double xPosAverage       = 0;
                    double yPosAverage       = 0;
                    double zPosAverage       = 0;
                    double minPosMagnitude   = 0;
                    double maxPosMagnitude   = 0;
                    int    minPosMagnitudeId = 0;
                    int    maxPosMagnitudeId = 0;

                    double velocityRMS       = 0;
                    double xVelAverage       = 0;
                    double yVelAverage       = 0;
                    double zVelAverage       = 0;
                    double minVelMagnitude   = 0;
                    double maxVelMagnitude   = 0;
                    int    minVelMagnitudeId = 0;
                    int    maxVelMagnitudeId = 0;

                    for (int k = 0; k < comparePoints->GetNumberOfPoints(); k++)
                    {
                        Vec3d compareVecPos      = Vec3d(comparePoints->GetPoint(k));
                        Vec3d verificationVecPos = Vec3d(verificationPoints->GetPoint(k));

                        double diffMagnitude = (verificationVecPos - compareVecPos).norm();

                        displacementRMS += (diffMagnitude * diffMagnitude);
                        xPosAverage     += abs(verificationVecPos.x() - compareVecPos.x());
                        yPosAverage     += abs(verificationVecPos.y() - compareVecPos.y());
                        zPosAverage     += abs(verificationVecPos.z() - compareVecPos.z());

                        if (diffMagnitude > maxPosMagnitudeId)
                        {
                            maxPosMagnitude   = diffMagnitude;
                            maxPosMagnitudeId = k;
                        }
                        if (diffMagnitude < minPosMagnitude)
                        {
                            minPosMagnitude   = diffMagnitude;
                            minPosMagnitudeId = k;
                        }

                        Vec3d compareVecVel      = Vec3d(compareVelocity->GetTuple(k));
                        Vec3d verificationVecVel = Vec3d(verificationVelocity->GetTuple(k));

                        diffMagnitude = (verificationVecVel - compareVecVel).norm();

                        velocityRMS += (diffMagnitude * diffMagnitude);
                        xVelAverage += abs(verificationVecVel.x() - compareVecVel.x());
                        yVelAverage += abs(verificationVecVel.y() - compareVecVel.y());
                        zVelAverage += abs(verificationVecVel.z() - compareVecVel.z());

                        if (diffMagnitude > maxVelMagnitudeId)
                        {
                            maxVelMagnitude   = diffMagnitude;
                            maxVelMagnitudeId = k;
                        }
                        if (diffMagnitude < minVelMagnitude)
                        {
                            minVelMagnitude   = diffMagnitude;
                            minVelMagnitudeId = k;
                        }
                    }

                    displacementRMS /= comparePoints->GetNumberOfPoints();
                    displacementRMS  = sqrt(displacementRMS);

                    velocityRMS /= comparePoints->GetNumberOfPoints();
                    velocityRMS  = sqrt(velocityRMS);

                    xPosAverage /= comparePoints->GetNumberOfPoints();
                    yPosAverage /= comparePoints->GetNumberOfPoints();
                    zPosAverage /= comparePoints->GetNumberOfPoints();

                    xVelAverage /= comparePoints->GetNumberOfPoints();
                    yVelAverage /= comparePoints->GetNumberOfPoints();
                    zVelAverage /= comparePoints->GetNumberOfPoints();

                    dataTracker->probe("DisplacementRMS", displacementRMS);
                    dataTracker->probe("AverageXDisplacement", xPosAverage);
                    dataTracker->probe("AverageYDisplacement", yPosAverage);
                    dataTracker->probe("AverageZDisplacement", zPosAverage);
                    dataTracker->probe("MinDisplacement", minPosMagnitude);
                    dataTracker->probe("MinDisplacementIndex", minPosMagnitudeId);
                    dataTracker->probe("MaxDisplacement", maxPosMagnitude);
                    dataTracker->probe("MaxDisplacementIndex", maxPosMagnitudeId);

                    dataTracker->probe("VelocityRMS", velocityRMS);
                    dataTracker->probe("AverageXVelocity", xVelAverage);
                    dataTracker->probe("AverageYVelocity", yVelAverage);
                    dataTracker->probe("AverageZVelocity", zVelAverage);
                    dataTracker->probe("MinVelocity", minVelMagnitude);
                    dataTracker->probe("MinVelocityIndex", minVelMagnitudeId);
                    dataTracker->probe("MaxVelocity", maxVelMagnitude);
                    dataTracker->probe("MaxVelocityIndex", maxVelMagnitudeId);
                    dataTracker->streamProbesToFile(std::stod(std::string(compareMb->GetMetaData(i)->Get(vtkMultiBlockDataSet::NAME())).substr(9)));
                }

                if (objectName.empty())
                {
                    break;
                }
            }
        }
    }

    return;
}

void
EntityToVTK::compareMultiBlock(std::string multiblockFilePath, std::string objectName, std::string outputDir)
{
    vtkSmartPointer<vtkMultiBlockDataSet> vtkmb       = this->readvtkMultiBlock(multiblockFilePath);
    auto                                  dataTracker = std::make_shared<DataTracker>();
    dataTracker->setFilename(outputDir);
    dataTracker->configureProbe("X Displacement RMS");
    dataTracker->configureProbe("Y Displacement RMS");
    dataTracker->configureProbe("Z Displacement RMS");

    if (vtkmb)
    {
        vtkSmartPointer<vtkMultiBlockDataSet> timestepCurrentMb;
        vtkSmartPointer<vtkMultiBlockDataSet> innerCurrentMb;
        vtkSmartPointer<vtkMultiBlockDataSet> timestepPreviousMb;
        vtkSmartPointer<vtkMultiBlockDataSet> innerPreviousMb;

        vtkSmartPointer<vtkPolyData> currentPolydata;
        vtkSmartPointer<vtkPolyData> previousPolydata;

        vtkSmartPointer<vtkPoints> currentPoints;
        vtkSmartPointer<vtkPoints> previousPoints;

        vtkSmartPointer<vtkPointData> currentPointData;
        vtkSmartPointer<vtkPointData> previousPointData;

        for (unsigned int i = 1; i < vtkmb->GetNumberOfBlocks(); i++)
        {
            timestepPreviousMb = vtkMultiBlockDataSet::SafeDownCast(vtkmb->GetBlock(i - 1));
            timestepCurrentMb  = vtkMultiBlockDataSet::SafeDownCast(vtkmb->GetBlock(i));

            for (unsigned int j = 0; j < timestepCurrentMb->GetNumberOfBlocks(); j++)
            {
                if (objectName.empty() || strcmp(timestepCurrentMb->GetMetaData((unsigned int) j)->Get(vtkMultiBlockDataSet::NAME()), objectName.c_str()) == 0)
                {
                    innerCurrentMb  = vtkMultiBlockDataSet::SafeDownCast(timestepCurrentMb->GetBlock(j));
                    innerPreviousMb = vtkMultiBlockDataSet::SafeDownCast(timestepPreviousMb->GetBlock(j));

                    if (strcmp(innerCurrentMb->GetMetaData((unsigned int) 0)->Get(vtkMultiBlockDataSet::NAME()), "Vertex Info") != 0
                        || strcmp(innerPreviousMb->GetMetaData((unsigned int) 0)->Get(vtkMultiBlockDataSet::NAME()), "Vertex Info") != 0)
                    {
                        LOG(WARNING) << "warning: Missing vertex information in multiblock. Stoping comparision";
                        return;
                    }

                    currentPolydata  = vtkPolyData::SafeDownCast(innerCurrentMb->GetBlock(0));
                    previousPolydata = vtkPolyData::SafeDownCast(innerPreviousMb->GetBlock(0));

                    currentPoints  = currentPolydata->GetPoints();
                    previousPoints = previousPolydata->GetPoints();

                    currentPointData  = currentPolydata->GetPointData();
                    previousPointData = previousPolydata->GetPointData();

                    auto currentDisplacement = vtkDoubleArray::SafeDownCast(currentPointData->GetAbstractArray("Displacement"));

                    double displacementXRMS = 0;
                    double displacementYRMS = 0;
                    double displacementZRMS = 0;

                    for (int k = 0; k < currentPoints->GetNumberOfPoints(); k++)
                    {
                        Vec3d currentVecPos  = Vec3d(currentPoints->GetPoint(k));
                        Vec3d previousVecPos = Vec3d(previousPoints->GetPoint(k));

                        Vec3d recordedVecDisplacement   = currentVecPos - Vec3d(currentDisplacement->GetTuple(k));
                        Vec3d calculatedVecDisplacement = currentVecPos - previousVecPos;

                        displacementXRMS += (recordedVecDisplacement.x() - calculatedVecDisplacement.x()) * (recordedVecDisplacement.x() - calculatedVecDisplacement.x());
                        displacementYRMS += (recordedVecDisplacement.y() - calculatedVecDisplacement.y()) * (recordedVecDisplacement.y() - calculatedVecDisplacement.y());
                        displacementZRMS += (recordedVecDisplacement.z() - calculatedVecDisplacement.z()) * (recordedVecDisplacement.z() - calculatedVecDisplacement.z());
                    }

                    displacementXRMS /= currentPoints->GetNumberOfPoints();
                    displacementYRMS /= currentPoints->GetNumberOfPoints();
                    displacementZRMS /= currentPoints->GetNumberOfPoints();

                    displacementXRMS = sqrt(displacementXRMS);
                    displacementYRMS = sqrt(displacementYRMS);
                    displacementZRMS = sqrt(displacementZRMS);

                    dataTracker->probe("X Displacement RMS", displacementXRMS);
                    dataTracker->probe("Y Displacement RMS", displacementYRMS);
                    dataTracker->probe("Z Displacement RMS", displacementZRMS);
                    dataTracker->streamProbesToFile(std::stod(std::string(vtkmb->GetMetaData(i)->Get(vtkMultiBlockDataSet::NAME())).substr(9)));
                }

                if (objectName.empty())
                {
                    break;
                }
            }
        }
    }

    return;
}

vtkSmartPointer<vtkMultiBlockDataSet>
EntityToVTK::readvtkMultiBlock(std::string filepath)
{
    vtkXMLMultiBlockDataReader* reader = vtkXMLMultiBlockDataReader::New();
    reader->SetFileName(filepath.c_str());
    reader->Update();
    return vtkMultiBlockDataSet::SafeDownCast(reader->GetOutput());
}

void
EntityToVTK::setRecordingType(RecordingType type, int frequency)
{
    m_recordingType      = type;
    m_recordingFrequency = frequency;
}

void
EntityToVTK::setRecordingType(RecordingType type, std::vector<int> indexToRecord)
{
    m_recordingType = type;
    m_subsetIds     = indexToRecord;
}

void
EntityToVTK::setRecordingType(RecordingType type, std::vector<Vec3d> closestPoints)
{
    m_recordingType = type;
    m_closestPoints = closestPoints;
}

void
EntityToVTK::setRecordingType(RecordingType type, Vec3d lowerCorner, Vec3d upperCorner)
{
    m_recordingType   = type;
    m_lowerCornerBbox = lowerCorner;
    m_upperCornerBbox = upperCorner;
}
} // namespace imstk
