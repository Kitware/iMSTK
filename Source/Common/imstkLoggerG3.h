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

#include <g3log/logmessage.hpp>
#include <g3log/logworker.hpp>

namespace imstk
{
///
/// \struct stdSink
///
/// \brief A standard sink that prints the message to a standard output
///
struct stdSink
{
    // Linux xterm color
    // http://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
    enum class FontColor
    {
        Yellow = 33,
        Red = 31,
        Green = 32,
        White = 97
    };

    FontColor GetColor(const LEVELS level) const;
    void ReceiveLogMessage(g3::LogMessageMover logEntry);
};

using FileSinkHandle   = g3::SinkHandle<g3::FileSink>;
using StdoutSinkHandle = g3::SinkHandle<stdSink>;

#define IMSTK_CHECK(expr) \
    if (!expr)throw std::runtime_error("an error");

///
/// \struct Logger
///
/// \brief lazy initialized singleton
///
class LoggerG3
{
public:

    ///
    /// \brief Gets logger instances without sinks, creates if
    /// doesn't exist yet
    ///
    static LoggerG3& getInstance();

    ///
    /// \brief Starts logger with default sinks, use getInstance to
    /// create a logger with no sinks
    ///
    static LoggerG3& startLogger();

    // Disable copy & move constructors & assignments
    LoggerG3(const LoggerG3&) = delete;
    LoggerG3& operator=(const LoggerG3&) = delete;
    LoggerG3(LoggerG3&&) = delete;
    LoggerG3& operator=(LoggerG3&&) = delete;

    ///
    /// \brief Add a sink that logs to standard output
    ///
    std::unique_ptr<StdoutSinkHandle> addStdoutSink();

    ///
    /// \brief Add a sink that logs to file
    ///
    std::unique_ptr<FileSinkHandle> addFileSink(const std::string& name, const std::string& path = "./");

    ///
    /// \brief Add your own sink
    ///
    template<typename T, typename DefaultLogCall>
    std::unique_ptr<g3::SinkHandle<T>> addSink(std::unique_ptr<T> real_sink, DefaultLogCall call);

    ///
    /// \brief Create and initialize the logger
    ///
    void initialize();

    ///
    /// \brief Manual destruction of the logger members
    ///
    void destroy();

private:

    LoggerG3();

    std::shared_ptr<g3::LogWorker> m_g3logWorker;
};

template<typename T, typename DefaultLogCall>
std::unique_ptr<g3::SinkHandle<T>>
imstk::LoggerG3::addSink(std::unique_ptr<T> real_sink, DefaultLogCall call)
{
    return m_g3logWorker->addSink(std::move(real_sink), call);
}
} // namespace imstk
