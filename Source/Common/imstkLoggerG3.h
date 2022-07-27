/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <g3log/logmessage.hpp>
#include <g3log/logworker.hpp>

#include <memory>

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
