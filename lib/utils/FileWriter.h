#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <fstream>
#include <string>
#include <mutex>
#include <stdexcept>
#include <sys/stat.h>  // for file size (cross‑platform fallback)

#ifdef _WIN32
    #include <io.h>
    #define FSTAT _stat
    #define FSTAT_FUNC _stat
#else
    #include <unistd.h>
    #define FSTAT stat
    #define FSTAT_FUNC stat
#endif

class FileWriter {
public:
    // -----------------------------------------------------------------
    // Construction / Destruction
    // -----------------------------------------------------------------
    explicit FileWriter(const std::string& path = "", bool append = true) : m_path(path), m_append(append), m_isOpen(false) {
        if (!path.empty()) 
        {
            open(path, append);
        }
    }

    ~FileWriter() 
    {
        close();
    }

    // Disable copy (but allow move)
    FileWriter(const FileWriter&) = delete;
    FileWriter& operator=(const FileWriter&) = delete;

    FileWriter(FileWriter&& other) noexcept : m_path(std::move(other.m_path)), m_append(other.m_append), m_isOpen(other.m_isOpen), m_fileStream(std::move(other.m_fileStream)) 
    { 
        other.m_isOpen = false;
    }
    FileWriter& operator=(FileWriter&& other) noexcept 
    {
        if (this != &other) 
        {
            close();
            m_path = std::move(other.m_path);
            m_append = other.m_append;
            m_isOpen = other.m_isOpen;
            m_fileStream = std::move(other.m_fileStream);
            other.m_isOpen = false;
        }
        return *this;
    }

    // -----------------------------------------------------------------
    // Public interface
    // -----------------------------------------------------------------
    bool open(const std::string& path, bool append = true) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        close();  // close any previously opened file

        m_path = path;
        m_append = append;

        auto mode = append ? (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc);
        m_fileStream.open(path, mode);
        m_isOpen = m_fileStream.is_open();
        return m_isOpen;
    }

    void close() 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_fileStream.is_open()) 
        {
            m_fileStream.flush();
            m_fileStream.close();
        }
        m_isOpen = false;
    }

    // Write raw string (no newline appended)
    bool write(const std::string& text) 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isOpen)
        {
            return false;
        }
        m_fileStream << text;
        return !m_fileStream.fail();
    }

    // Write with newline
    bool writeln(const std::string& text) 
    {
        return write(text + "\n");
    }

    // Flush to disk
    bool flush() 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isOpen) return false;
        m_fileStream.flush();
        return !m_fileStream.fail();
    }

    // Check if file is open
    bool isOpen() const 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_isOpen && m_fileStream.is_open();
    }

    // Get file path
    std::string getPath() const 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_path;
    }

    // Get current file size in bytes (0 if not open or error)
    size_t getSize() const 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_isOpen) return 0;

        struct FSTAT st;
        if (FSTAT_FUNC(m_path.c_str(), &st) != 0) 
        {
            return 0;
        }
        return static_cast<size_t>(st.st_size);
    }

    // Check if the underlying stream is in a good state
    bool good() const 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_isOpen && m_fileStream.good();
    }

private:
    mutable std::mutex m_mutex;
    std::string m_path;
    bool m_append;
    bool m_isOpen;
    std::ofstream m_fileStream;
};

#endif // FILEWRITER_H