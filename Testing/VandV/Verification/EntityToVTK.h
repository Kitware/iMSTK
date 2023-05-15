/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#pragma once

#include "imstkPbdObject.h"
#include "imstkDynamicObject.h"
#include "imstkSceneObject.h"
#include "imstkPointSet.h"
#include "imstkAnalyticalGeometry.h"

#include <vtkMultiBlockDataSet.h>
#include <vtkSmartPointer.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>

namespace imstk
{
///
/// \class EntityToVTK
///
/// \brief Base class for outputing select entity data into a VTK multiblock dataset
///
class EntityToVTK
{
public:
    enum class RecordingType { NoRecording = 0, AllRecoding, FrequencyRecording, SubsetRecording, VectorRecording, BoundingBox };
    EntityToVTK() {};
    ~EntityToVTK() {};

    ///
    /// \brief Records the current state of tracked objects
    /// \param timeStep dt of the system
    ///
    void recordObjectState(double timeStep);

    ///
    /// \brief Write the entire VTK multiblock out
    /// \param fileName path to be written to
    ///
    void writeObjectsToFile(std::string fileName);

    ///
    /// \brief Write the entire VTK multiblock as a time series
    /// Ensure setTimeEriesFilePath is set before calling
    ///
    void writeTimeSeriesJsonFile();

    ///
    /// \brief Add entitiy to list of object to record
    /// \param entity entity to record
    ///
    void addEntity(std::shared_ptr<Entity> entity);

    ///
    /// \brief Set the time between recording.
    /// The timeSteps from recordObjectState are summed until the value is greater than m_timeBetweenRecordings.
    ///
    void setTimeBetweenRecordings(double timeBetweenRecordings) { this->m_timeBetweenRecordings = timeBetweenRecordings; }

    ///
    /// \brief Set the file path for time series
    /// \param filepath path for time series to be written to
    ///
    void setTimeSeriesFilePath(std::string filepath) { this->m_timeSeriesFilepath = filepath; }

    ///
    /// \brief Enable or disable the writing vertex information, when disabled the information will not be written out to the time series
    ///@{
    void enableWritingVertexInformation() { this->m_writeVertexInfo = true; }
    void disableWritingVertexInformation() { this->m_writeVertexInfo = false; }
    ///@}

    ///
    /// \brief Enable or disable the writing physical geometry, when disabled the geometry will not be written out to the time series
    ///@{
    void enableWritingPhysicalGeometry() { this->m_writePhysicalGeo = true; }
    void disableWritingPhysicalGeometry() { this->m_writePhysicalGeo = false; }
    ///@}

    ///
    /// \brief Enable or disable the writing visual geometry, when disabled the geometry will not be written out to the time series
    ///@{
    void enableWritingVisualGeometry() { this->m_writeVisualGeo = true; }
    void disableWritingVisualGeometry() { this->m_writeVisualGeo = false; }
    ///@}

    ///
    /// \brief Enable or disable the writing collision geometry, when disabled the geometry will not be written out to the time series
    ///@{
    void enableWritingCollisionGeometry() { this->m_writeCollisionGeo = true; }
    void disableWritingCollisionGeometry() { this->m_writeCollisionGeo = false; }
    ///@}

    ///
    /// \brief Compare the position and velocity of each vertex from two vtk polydatas
    ///
    void compareMultiBlocks(std::string compareFilePath, std::string verificationFilePath, std::string objectName, std::string outputDir);

    ///
    /// \brief Compare the position of each vertex from one vtk polydata
    ///
    void compareMultiBlock(std::string multiblockFilePath, std::string objectName, std::string outputDir);

    ///
    /// \brief Set recording type to be used
    ///
    void setRecordingType(RecordingType type) { this->m_recordingType = type; }
    void setRecordingType(RecordingType type, int frequency);
    void setRecordingType(RecordingType type, std::vector<int> indexToRecord);
    void setRecordingType(RecordingType type, std::vector<Vec3d> closestPoints);
    void setRecordingType(RecordingType type, Vec3d lowerCorner, Vec3d upperCorner);

    ///
    /// \brief Set recording type to be used
    ///
    RecordingType getRecordingType() { return this->m_recordingType; }

    ///
    /// \brief Enable or disable the constraint counting
    ///@{
    void enableConstraintCounting() { this->m_enableConstraintCounting = true; }
    void disableConstraintCounting() { this->m_enableConstraintCounting = false; }
///@}

protected:
    ///
    /// \brief Convert iMSTK entity to VTK multiblock
    /// \param entity entity to convert
    ///
    vtkSmartPointer<vtkMultiBlockDataSet> convertToMultiBlock(std::shared_ptr<Entity> entity);

    ///
    /// \brief Extract position, velocity, displacement, and constraint out from entity's vertices
    ///
    vtkSmartPointer<vtkPolyData> getVertexInformation(std::shared_ptr<Entity> entity);

    ///
    /// \brief Extract vertex information from a point set
    ///
    vtkSmartPointer<vtkPolyData> getVertexInformation(std::shared_ptr<PointSet> pointSet, std::shared_ptr<PbdBody> body, std::string name);

    ///
    /// \brief Extract vertex information from an analytical geometry
    ///
    vtkSmartPointer<vtkPolyData> getVertexInformation(std::shared_ptr<AnalyticalGeometry> analyticalGeo, std::shared_ptr<PbdBody> body, std::string name);

    ///
    /// \brief Extract different geometry types from entity. Uses geometryToData function once correct geometry type is found
    ///@{
    vtkSmartPointer<vtkDataObject> getPhysicalGeometry(std::shared_ptr<Entity> entity);
    vtkSmartPointer<vtkDataObject> getVisualGeometry(std::shared_ptr<Entity> entity);
    vtkSmartPointer<vtkDataObject> getCollisionGeometry(std::shared_ptr<Entity> entity);
    vtkSmartPointer<vtkDataObject> getGhost(std::shared_ptr<Entity> entity);
    ///@}

    ///
    /// \brief Convert geometry to vtkDataObject. Uses imstk GeometryUtilities for conversion
    ///
    vtkSmartPointer<vtkDataObject> geometryToData(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Count number of constraints on each vertex and add to polydata
    /// \param pbdObject pbdObject to count contraints from
    /// \param polydata polydata to add constrait count to
    ///
    void addConstraintCount(std::shared_ptr<PbdObject> pbdObject, vtkSmartPointer<vtkPolyData> polydata);

    ///
    /// \brief Check name of block and return if it should be added to time series
    /// \param name name of block from vtkMultiblock
    ///
    bool checkType(std::string name);

    ///
    /// \brief Write out multiblock as part of the time series
    /// \param mb vtkMultiblock to write out
    /// \param timestep timestep when multiblock was recorded
    ///
    void writeObjectsToTimeSeries(vtkSmartPointer<vtkMultiBlockDataSet> mb, double timestep);

    ///
    /// \brief Read vtkMultiBlockDataSet at given filepath
    ///
    vtkSmartPointer<vtkMultiBlockDataSet> readvtkMultiBlock(std::string filepath);

    ///
    /// \brief Setup m_indexToRecord based on m_recordingType
    ///
    void setupIndexToRecord(std::shared_ptr<Entity> entity);

    ///
    /// \brief Find the closest points in entity to each point in m_closestPoints
    ///
    void findClosestPoints(std::shared_ptr<PointSet> pointSet, std::string name);

    ///
    /// \brief Find points in bbox defined by m_lowerCornerBbox and m_upperCornerBbox
    ///
    void findEnclosedPoints(std::shared_ptr<PointSet> pointSet, std::string name);

protected:
    std::map<std::string, std::shared_ptr<Entity>> m_Entities;
    std::vector<double> m_timesteps;
    vtkSmartPointer<vtkMultiBlockDataSet> m_outputBlock = vtkSmartPointer<vtkMultiBlockDataSet>::New();
    int           m_blockCount = 0;
    double        m_timeSinceLastRecording = -1;
    double        m_timeBetweenRecordings  = 0;
    double        m_accumlatedTime    = 0;
    bool          m_writeVertexInfo   = false;
    bool          m_writePhysicalGeo  = true;
    bool          m_writeVisualGeo    = false;
    bool          m_writeCollisionGeo = false;
    bool          m_writeGhost = true;
    bool          m_initializedTimeSeries = false;
    std::string   m_timeSeriesFilepath    = "";
    std::string   m_timeSeriesJson     = "";
    RecordingType m_recordingType      = RecordingType::AllRecoding;
    int           m_recordingFrequency = 1;
    std::map<std::string, std::vector<int>> m_indexToRecord;
    std::vector<Vec3d> m_closestPoints;
    std::map<std::string, std::map<int, int>> m_lastConstraintCounts;
    std::map<std::string, int> m_lastConstraintTotals;
    bool  m_enableConstraintCounting = true;
    Vec3d m_lowerCornerBbox;
    Vec3d m_upperCornerBbox;
    std::vector<int> m_subsetIds;
};
} // namespace imstk
