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
class DataTracker
{
public:

    struct ePhysics
    {
        static constexpr int SolverTime_ms  = 0;
        static constexpr int NumConstraints = 1;
        static constexpr int AverageC       = 2;
    };
    struct Physics
    {
        static constexpr char const* SolverTime_ms  = "SolverTime_ms";
        static constexpr char const* NumConstraints = "NumConstraints";
        static constexpr char const* AverageC       = "AverageC";
    };

    DataTracker();
    ~DataTracker();
    enum class eDecimalFormat_Type { SystemFormatting = 0, DefaultFloat, FixedMantissa, SignificantDigits };

    void useTabDelimiter() { m_delimiter = '\t'; }
    void useCommaDelimiter() { m_delimiter = ','; }
    void setFilename(const std::string& fn) { m_filename = fn; }

    StopWatch& getStopWatch(int idx);
    void probe(int idx, double value);
    int probe(const std::string& name, double value);
    void probeElapsedTime_s(int idx);
    int configureProbe(const std::string& name, std::streamsize precision = 3, eDecimalFormat_Type notation = eDecimalFormat_Type::SystemFormatting);

    // Note, if you are going to mix probing via index and name, use a negative number, as auto indexing uses positive
    int configureProbe(const std::string& name, int index, std::streamsize precision = 6, eDecimalFormat_Type notation = eDecimalFormat_Type::SystemFormatting);

    double getValue(int idx);
    double getValue(const std::string& name);
    std::string getName(int idx);

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
