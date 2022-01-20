/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

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