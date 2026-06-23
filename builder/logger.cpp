#include "logger.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_Level)
    {
        return;
    } 

    std::lock_guard<std::mutex> lock(m_Mutex);

    std::string logLine = getTimestamp() + " [" + levelToString(level) + "] " + message;

    // console out
    if (m_ConsoleOutput) 
    {
        auto& out = (level >= LogLevel::ERROR) ? std::cerr : std::cout;
        out << logLine << std::endl;
    }

    // Output to file
    if (m_FileWriter && m_FileWriter->isOpen()) 
    {
        m_FileWriter->writeln(logLine);
        m_FileWriter->flush();  // optional stuff - ensures crash safety
    }
}

std::string Logger::getTimestamp() 
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::levelToString(LogLevel& level) 
{
    switch (level) 
    {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}


void Logger::debug(const std::string& msg)   
{
    log(LogLevel::DEBUG, msg); 
}

void Logger::info(const std::string& msg)    
{
    log(LogLevel::INFO,  msg); 
}

void Logger::warn(const std::string& msg)    
{
    log(LogLevel::WARN,  msg); 
}

void Logger::error(const std::string& msg)   
{
    log(LogLevel::ERROR, msg); 
}

void Logger::fatal(const std::string& msg)   
{
    log(LogLevel::FATAL, msg); 
}

void Logger::setLevel(LogLevel& logLevel)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Level = logLevel;
}

void Logger::enableConsole(bool& enabled)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_ConsoleOutput = enabled;
}

void Logger::setLogFile(const std::string& path, bool append) 
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (path.empty()) 
    {
        m_FileWriter.reset();
    } 
    else 
    {
        if (!m_FileWriter) 
        {
            m_FileWriter = std::make_unique<FileWriter>();
        }
        m_FileWriter->open(path, append);
    }
}