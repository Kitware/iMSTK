/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <stdexcept>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>

#include "imstkSpinLock.h"

// Using g3log loglevels for compatibility
#include "g3log/loglevels.hpp"
#include <deque>
/*

Still some weirdness, as we are still building G3 Log, the defines are still active

#define DEBUG 0
#define INFO 1
#define WARNING 2
#define FATAL 3
*/
namespace imstk
{
#if defined(__func__)
#define IMSTK_CURRENT_FUNCTION __func__
#elif defined(__FUNCSIG__)
#define IMSTK_CURRENT_FUNCTION __FUNCSIG__
#elif defined(__PRETTY_FUNCTION__)
#define IMSTK_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCTION__)
#define IMSTK_CURRENT_FUNCTION __FUNCTION__
#else
#define IMSTK_CURRENT_FUNCTION "???"
#endif

#define IMSTK_MAKE_STRING(x)  #x

class AssertionFailure : public std::runtime_error
{
public:
    /// Constructor
    /// \param message Exception message
    explicit AssertionFailure(const std::string& message) : std::runtime_error(message)
    {
    }
};

class LogOutput
{
public:
    LogOutput() = default;
    virtual ~LogOutput() = default;

    /// \param message to be written out
    /// \return true on success
    virtual bool writeMessage(const std::string& message) = 0;
};

class NullOutput : public LogOutput
{
public:
    virtual bool writeMessage(const std::string&) { return true; }
};

/*
/// Class to output logging information to a give file
class FileOutput : public LogOutput
{
public:

    /// Constructor
    /// \param filename The filename to be used for writing
    explicit FileOutput(const std::string& filename);

    /// \param message to be written out
    /// \return true on success
    bool writeMessage(const std::string& message) override;

private:
    std::string m_filename;
    std::ofstream m_stream;
    boost::mutex m_mutex;
};
*/

/// Class to output logging information to a stream that can be passed
/// into the constructor of the class
class StreamOutput : public LogOutput
{
public:

    /// Constructor
    /// \param ostream stream to be used for writing
    /// ostream parameter to be passed by non-const reference on purpose.
    explicit StreamOutput(std::ostream& ostream); //NOLINT

    /// Writes a message to the stream.
    /// \param	message	Message to be written to the stream
    /// \return	True on success
    bool writeMessage(const std::string& message) override;

private:
    std::ostream& m_stream;
    ParallelUtils::SpinLock m_mutex;
};

/// Class to output logging information to a stream that can be passed
/// into the constructor of the class
class CacheOutput : public LogOutput
{
public:
    CacheOutput();
    /// Writes a message to the stream.
    /// \param	message	Message to be written to the stream
    /// \return	True on success
    bool writeMessage(const std::string& message) override;

    bool hasMessages() const;

    std::string popLastMessage();

private:
    std::ofstream m_outFile;
    std::shared_ptr<StreamOutput>   m_fileOutput;
    std::deque<std::string>         m_messages;
    ParallelUtils::SpinLock mutable m_mutex;
};

class LoggerSynchronous
{
public:
    /// Constructor.
    /// \param output The LogOutput instance used to display or log the data.
    LoggerSynchronous(std::shared_ptr<LogOutput> output) : m_threshold(DEBUG.value), m_name("imstk"), m_output(output) { }

    static std::shared_ptr<LoggerSynchronous> instance()
    {
        static std::shared_ptr<LoggerSynchronous> defaultLogger = std::make_shared<LoggerSynchronous>(
            std::make_shared<CacheOutput>());
        return defaultLogger;
    }

    /// Destructor.
    ~LoggerSynchronous() = default;

    /// For compatibility
    static void startLogger() {}

    /// Uses the contained instance of LogOutput to write the log message
    /// \return true on success
    /// \param message the message to be printed
    bool writeMessage(const std::string& message)
    {
        return m_output->writeMessage(message);
    }

    /// Gets the logging threshold.
    /// Anything message with less than this level will be ignored.
    /// \return The threshold value.
    int getThreshold() const
    {
        return m_threshold;
    }

    /// Sets the logging threshold.
    /// Anything message with less than this level will be ignored.
    /// \param val The value to be used as the threshold.
    void setThreshold(int val)
    {
        m_threshold = val;
    }

    /// Gets the output object used by this logger.
    /// \return The current output object used this logger.
    std::shared_ptr<LogOutput> getOutput() const
    {
        return m_output;
    }

    /// TODO HS-2022-06-03 There is some issue in Unity where the cast
    /// to CacheOutput isn't working, this is a patch to get around that
    std::shared_ptr<CacheOutput> getCacheOutput() const
    {
        return std::dynamic_pointer_cast<CacheOutput>(m_output);
    }

    /// Sets the output object used by this logger.
    /// \param val The output object to be used.
    void setOutput(std::shared_ptr<LogOutput> val)
    {
        m_output = val;
    }

    std::string getName()
    {
        return m_name;
    }

private:
    int m_threshold;
    std::string m_name;
    std::shared_ptr<LogOutput> m_output;
};

class LogMessageBase
{
public:

    /// Construct a LogMessage
    /// \param logger The logger to be used
    /// \param level The logging level for this message
    LogMessageBase(LoggerSynchronous* logger, int level);

    ~LogMessageBase() noexcept(false) {
        flush();
    };

    /// Add the given input to the current log message.
    /// \param input The input to be added to the current stream
    template<typename T>
    LogMessageBase& operator <<(T&& input)
    {
        m_stream << input;
        return *this;
    }

    // A specialization for output manipulators (functions that apply to the stream).
    // Otherwise overloaded manipulators like std::endl and std::endl don't work, since the compiler can't know
    // what overloaded variant to apply.
    LogMessageBase& operator <<(std::ios_base& (*manipulator)(std::ios_base&))
    {
        m_stream << *manipulator;
        return *this;
    }

    // A specialization for output manipulators (functions that apply to the stream).
    // Otherwise overloaded manipulators like std::hex and std::endl don't work, since the compiler can't know
    // what overloaded variant to apply.
    LogMessageBase& operator <<(std::ostream& (*manipulator)(std::ostream&))
    {
        m_stream << *manipulator;
        return *this;
    }

protected:
    /// \return the current content of the message to be logged
    std::string getMessage()
    {
        return m_stream.str();
    }

    /// write the current message to the logger
    void flush()
    {
        m_logger->writeMessage(m_stream.str());
    }

private:
    std::ostringstream m_stream;
    LoggerSynchronous* m_logger;
};

class AssertMessage : public LogMessageBase
{
public:
    /// The type used for the callback function that is triggered after an assertion has failed.
    typedef void (* DeathCallback)(const std::string& message);

    /// Constructor.
    /// \param logger %Logger used to log this message.
    explicit AssertMessage(LoggerSynchronous* logger) : LogMessageBase(logger, FATAL.value)
    {
    }

    /// Constructor.
    /// \param logger %Logger used to log this message.
    explicit AssertMessage(const std::unique_ptr<LoggerSynchronous>& logger) : LogMessageBase(logger.get(), FATAL.value)
    {
    }

    /// Constructor.
    /// \param logger %Logger used to log this message.
    explicit AssertMessage(const std::shared_ptr<LoggerSynchronous>& logger) : LogMessageBase(logger.get(), FATAL.value)
    {
    }

    ~AssertMessage() noexcept(false)
    {
        flush();
        m_killMeNow(getMessage());
    }

    /// After an assertion has failed, call some arbitrary function.
    /// The callback function should cause the application (or at least the current thread) to terminate.
    ///
    /// Thread-unsafe if called concurrently from multiple threads, or concurrently with a failing assertion.
    static void setFailureCallback(DeathCallback callback);

    /// Get the callback that will currently be called after an assertion has failed.
    /// Thread-unsafe if called concurrently from multiple threads, or concurrently with a failing assertion.
    /// \return The callback.
    static DeathCallback getFailureCallback();

    /// After an assertion has failed, throw a C++ exception.
    /// Thread-unsafe if called concurrently from multiple threads, or concurrently with a failing assertion.
    static void setFailureBehaviorToThrow()
    {
        setFailureCallback(throwException);
    }

    /// After an assertion has failed, enter the debugger or kill the application in a system-dependent way.
    /// Thread-unsafe if called concurrently from multiple threads, or concurrently with a failing assertion.
    static void setFailureBehaviorToDeath()
    {
        setFailureCallback(killApplication);
    }

private:
    /// Kill the application by throwing an exception.
    /// \param errorMessage Message describing the error.
    static void throwException(const std::string& errorMessage);

    /// Enter the debugger or kill the application in a system-dependent way.
    /// \param errorMessage Message describing the error (which will be ignored).
    static void killApplication(const std::string& errorMessage);

    /// The callback function that is triggered after an assertion has failed.
    /// Thread-unsafe if called concurrently from multiple threads.
    static DeathCallback m_killMeNow;
};
} // namespace imstk

#define LOG(level)                                                              \
    if (level.value < (::imstk::LoggerSynchronous::instance())->getThreshold()) \
    {                                                                           \
    }                                                                           \
    else                                                                        \
    /* important: no curly braces around this! */                               \
    ::imstk::LogMessageBase(::imstk::LoggerSynchronous::instance().get(), level.value)

#define LOG_IF(level, condition)                  \
    if (!(condition))                             \
    {                                             \
    }                                             \
    else                                          \
    /* important: no curly braces around this! */ \
    LOG(level)

#define CHECK(condition)                                                                                \
    if ((condition))                                                                                    \
    {                                                                                                   \
    }                                                                                                   \
    else                                                                                                \
    /* important: no curly braces around this! */                                                       \
    ::imstk::AssertMessage(::imstk::LoggerSynchronous::instance().get()) << "*** Assertion failed: " << \
        IMSTK_MAKE_STRING(condition) << " ***" << std::endl <<                                          \
        "    in " << IMSTK_CURRENT_FUNCTION << std::endl <<                                             \
        "    at " << __FILE__ << ":" << __LINE__ << std::endl
