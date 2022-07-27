/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <memory>
#include <string>

namespace imstk
{
class TaskGraph;

///
/// \class TaskGraphVizWriter
///
/// \brief Writes a TaskGraph to an svg file. Produces unique node names from duplicates with postfix.
/// Can also color by node compute time and highlight the critical path
///
class TaskGraphVizWriter
{
public:
    TaskGraphVizWriter() = default;
    virtual ~TaskGraphVizWriter() = default;

public:
    ///
    /// \brief The graph to write
    ///
    void setInput(std::shared_ptr<TaskGraph> graph) { this->m_inputGraph = graph; }

    ///
    /// \brief The filename and path to write too
    ///
    void setFileName(std::string fileName) { this->m_fileName = fileName; }

    ///
    /// \brief If on, will highlight the critical path in red
    ///
    void setHighlightCriticalPath(bool highlightCriticalPath) { this->m_highlightCriticalPath = highlightCriticalPath; }

    ///
    /// \brief If on, will write the time the node took to complete as a color
    ///
    void setWriteNodeComputeTimesColor(bool writeNodeComputeTimesColor) { this->m_writeNodeComputeTimesColor = writeNodeComputeTimesColor; }

    ///
    /// \brief If on, will write the time the node took to complete in name as text
    ///
    void setWriteNodeComputeTimesText(bool writeNodeComputeTimesText) { this->m_writeNodeComputeTimesText = writeNodeComputeTimesText; }

    std::shared_ptr<TaskGraph> getInput() const { return m_inputGraph; }
    const std::string& getFileName() const { return m_fileName; }
    bool getHighlightCriticalPath() const { return m_highlightCriticalPath; }
    bool getWriteNodeComputeTimesColor() const { return m_writeNodeComputeTimesColor; }
    bool getWriteNodeComputeTimesText() const { return m_writeNodeComputeTimesText; }

    ///
    /// \brief Writes the graph to a file given the filename
    ///
    void write();

private:
    std::shared_ptr<TaskGraph> m_inputGraph = nullptr;
    std::string m_fileName            = "";
    bool m_highlightCriticalPath      = false;
    bool m_writeNodeComputeTimesColor = false;
    bool m_writeNodeComputeTimesText  = false;
};
} // namespace imstk