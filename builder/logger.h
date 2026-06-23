#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include "../lib/utils/FileWriter.h"

#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <memory>
#include <sstream>
#include <chrono>
#include <iomanip>

enum class LogLevel 
{
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3,
    FATAL = 4
};

class Logger
{
private:
    Logger() = default;
    LogLevel m_Level;
    std::unique_ptr<FileWriter> m_FileWriter;
    bool m_ConsoleOutput;
    std::mutex m_Mutex;

public:
    static Logger& getInstance();

    //config
    void setLevel(LogLevel& level);
    void enableConsole(bool& enabled);
    void setLogFile(const std::string& path, bool append = true); // might want to pass a fs::path later

    // Logs
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void fatal(const std::string& msg);

    // Delete copy/move for singleton
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    //Log
    template<typename... Args>
    void log(LogLevel level, const std::string& format, Args&&... args) 
    {
        char buffer[2048];
        snprintf(buffer, sizeof(buffer), format.c_str(), std::forward<Args>(args)...);
        log(level, std::string(buffer));
    }

    void log(LogLevel level, const std::string& message);

    //Helper funcs
    std::string getTimestamp();
    std::string levelToString(LogLevel& level);

};


#endif