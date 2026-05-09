#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>

// log levels
enum LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

bool logToFileEnabled = false;

// cololrs for console output
inline std::string getColor(LogLevel level)
{
    switch (level)
    {
    case TRACE:
        return "\033[36m"; // cyan
    case DEBUG:
        return "\033[34m"; // blue
    case INFO:
        return "\033[32m"; // green
    case WARN:
        return "\033[33m"; // yellow
    case ERROR:
        return "\033[31m"; // red
    case FATAL:
        return "\033[35m"; // magenta
    }
    return "\033[0m"; // default
}

// timestamp for log entries
inline std::string currentTime()
{
    std::time_t now = std::time(nullptr);
    char buf[100];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}

// logger
template <typename T>
void logger(LogLevel level, T message)
{
    std::string prefix;
    switch (level)
    {
    case TRACE:
        prefix = "[TRACE] ";
        break;
    case DEBUG:
        prefix = "[DEBUG] ";
        break;
    case INFO:
        prefix = "[INFO] ";
        break;
    case WARN:
        prefix = "[WARN] ";
        break;
    case ERROR:
        prefix = "[ERROR] ";
        break;
    case FATAL:
        prefix = "[FATAL] ";
        break;
    }

    std::ostringstream oss;
    oss << "[" << currentTime() << "] " << prefix << message;
    std::string finalMessage = oss.str();

    std::cerr << getColor(level) << finalMessage << "\033[0m" << std::endl;

    // Log to file only if enabled
    if (logToFileEnabled)
    {
        std::ofstream logfile("app.log", std::ios::app);
        if (logfile.is_open())
        {
            logfile << finalMessage << std::endl;
            logfile.flush();
        }
    }
}