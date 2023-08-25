/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDataTracker.h"
#include <cmath>

namespace imstk
{
DataTracker::DataTracker()
{
    m_nextIndex = 0;
    m_filename  = "";
    useCommaDelimiter();
}

DataTracker::~DataTracker()
{
    m_elements.clear();
    m_file.close();
}

int
DataTracker::configureProbe(const std::string& name, std::streamsize precision, eDecimalFormat_Type notation)
{
    auto& e = getElement(name);
    e.precision = precision;
    e.notation  = notation;
    return e.index;
}

int
DataTracker::configureProbe(const std::string& name, int index, std::streamsize precision, eDecimalFormat_Type notation)
{
    auto& e = getElement(index);
    e.name      = name;
    e.precision = precision;
    e.notation  = notation;
    return e.index;
}

int
DataTracker::probe(const std::string& name, double value)
{
    auto& e = getElement(name);
    e.value = value;
    return e.index;
}

void
DataTracker::probe(int index, double value)
{
    auto& e = getElement(index);
    e.value = value;
}

void
DataTracker::probeElapsedTime_s(int index)
{
    auto& e  = getElement(index);
    auto& sw = getStopWatch(index);
    e.value = sw.getTimeElapsed();
}

double
DataTracker::getValue(int idx)
{
    return getElement(idx).value;
}

double
DataTracker::getValue(const std::string& name)
{
    return getElement(name).value;
}

std::string
DataTracker::getName(int idx)
{
    return getElement(idx).name;
}

DataTracker::Element&
DataTracker::getElement(std::string const& name)
{
    for (Element& e : m_elements)
    {
        if (e.name == name)
        {
            return e;
        }
    }
    Element e;
    e.name  = name;
    e.index = m_nextIndex++;
    m_elements.push_back(e);
    return m_elements.back();
}

DataTracker::Element&
DataTracker::getElement(int idx)
{
    for (Element& e : m_elements)
    {
        if (e.index == idx)
        {
            return e;
        }
    }
    Element e;
    e.name  = "Unknown";
    e.index = idx;
    m_elements.push_back(e);
    return m_elements.back();
}

StopWatch&
DataTracker::getStopWatch(int idx)
{
    return m_timers[idx];
}

void
DataTracker::createFile()
{
    if (m_file.is_open())
    {
        m_file.close();
    }

    int idx = 0;
    m_file.open(m_filename, std::ofstream::out | std::ofstream::trunc);
    // Write our headers
    m_file << "Time(s)" << m_delimiter;
    for (Element& e : m_elements)
    {
        m_file << e.name;
        if ((++idx) < (m_elements.size()))
        {
            m_file << m_delimiter;
        }
    }
    m_file << std::endl;
    m_file.flush();
}

void
DataTracker::streamProbesToFile(double time)
{
    if (!m_file.is_open())
    {
        if (!m_filename.empty())
        {
            createFile();
        }
        else
        {
            return;
        }
    }

    double d;
    int    idx = 0;
    // Write out probe values in heading order
    m_file << std::fixed << std::setprecision(3) << time << m_delimiter;
    for (Element& e : m_elements)
    {
        d = e.value;
        if (d == 0)
        {
            m_file << std::fixed << std::setprecision(0);
            m_file << 0;
        }
        else if (d - ((int)d) == 0)
        {
            m_file << std::fixed << std::setprecision(0);
            m_file << d;
        }
        else if (std::isnan(d))
        {
            m_file << "-0.$";
        }
        else
        {
            switch (e.notation)
            {
            case eDecimalFormat_Type::SystemFormatting:
                m_file << std::fixed << std::setprecision(e.precision);
                break;
            case eDecimalFormat_Type::DefaultFloat:
                m_file << std::defaultfloat << std::setprecision(e.precision);
                break;
            case eDecimalFormat_Type::FixedMantissa:
                m_file << std::fixed << std::setprecision(e.precision);
                break;
            case eDecimalFormat_Type::SignificantDigits:
                m_file << std::scientific << std::setprecision(e.precision);
            }
            m_file << d;
        }
        if ((++idx) < (m_elements.size()))
        {
            m_file << m_delimiter;
        }
    }
    m_file << std::endl;
    m_file.flush();
}
} // namespace imstk
