/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <map>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "imstkTimer.h"

namespace imstk
{
///
/// \class DataTracker
///
/// \brief Store time based data to write to a file
///
class DataTracker
{
public:

    ///
    /// \brief Common data values to track
    ///
    struct ePhysics
    {
        static constexpr int SolverTime_ms  = 0;
        static constexpr int NumConstraints = 1;
        static constexpr int AverageC       = 2;
    };
    ///
    /// \brief Header names of the common data values to track
    ///
    struct Physics
    {
        static constexpr char const* SolverTime_ms  = "SolverTime_ms";
        static constexpr char const* NumConstraints = "NumConstraints";
        static constexpr char const* AverageC       = "AverageC";
    };

    DataTracker();
    ~DataTracker();
    ///
    /// \brief C++ I/O manipulation flags
    ///
    enum class eDecimalFormat_Type { SystemFormatting = 0, DefaultFloat, FixedMantissa, SignificantDigits };

    void useTabDelimiter() { m_delimiter = '\t'; }
    void useCommaDelimiter() { m_delimiter = ','; }
    void setFilename(const std::string& fn) { m_filename = fn; }

    ///
    /// \brief An available timer for each data item tracked
    ///
    StopWatch& getStopWatch(int idx);

    ///
    /// \brief Track a data value associated with a header id
    /// \param ID of the header to add to, This is the ID returned from configureProbe
    /// \param value to track
    ///
    void probe(int idx, double value);

    ///
    /// \brief Track a data value associated with a header string
    /// \param String of the header to add to
    /// \param value to track
    /// \return the idx ID for this header string
    ///
    int probe(const std::string& name, double value);

    ///
    /// \brief Track a time duration associated with a header
    /// \param ID of the header to add to, This is the ID returned from configureProbe
    ///
    void probeElapsedTime_s(int idx);

    ///
    /// \brief Create and configure a probe and how it is written to file
    /// \param header name
    /// \param C++ precision
    /// \param C++ I/O manipulation type
    /// \return ID of the probe created
    ///
    int configureProbe(const std::string& name, std::streamsize precision = 3, eDecimalFormat_Type notation = eDecimalFormat_Type::SystemFormatting);

    ///
    /// \brief Create and configure a probe and how it is written to file
    /// \param ID of the header added
    /// \param Index ID to associate with this header
    ///        Note, if you are going to mix probing via index and name, use a negative number, as auto indexing uses positive
    /// \param C++ precision
    /// \param C++ I/O manipulation type
    /// \return ID of the probe created, should be the same as the index provided
    ///
    int configureProbe(const std::string& name, int index, std::streamsize precision = 6, eDecimalFormat_Type notation = eDecimalFormat_Type::SystemFormatting);

    double getValue(int idx);
    double getValue(const std::string& name);
    std::string getName(int idx);

    ///
    /// \brief Write the current probe values to a row in the data file
    /// \param Current simulation time
    void streamProbesToFile(double time);

protected:
    struct Element
    {
        int index;
        std::string name;
        double value = std::numeric_limits<double>::quiet_NaN();
        std::streamsize precision    = 3;
        eDecimalFormat_Type notation = eDecimalFormat_Type::SystemFormatting;
    };

    std::string m_filename;
    char m_delimiter;
    std::vector<Element> m_elements;
    std::ofstream m_file;
    std::map<int, StopWatch> m_timers;
    int m_nextIndex;

    void createFile();
    Element& getElement(int idx);
    Element& getElement(std::string const& name);
};
} // namespace imstk
