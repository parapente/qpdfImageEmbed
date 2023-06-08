#include "logger.h"

Logger::Logger() { out = &std::cerr; }

Logger::Logger(std::string fileName) { open(fileName); }

void Logger::write(const std::string &value) {
    std::string text = value;

    if (m_showDate) {
        time_t rawTime;
        struct tm *timeInfo;
        char buffer[21];

        time(&rawTime);
        timeInfo = localtime(&rawTime);
        strftime(buffer, 21, "%F %T ", timeInfo);
        text = buffer + text;
    }

    *out << text << std::endl;
}

void Logger::setShowDateTime(bool value) { m_showDate = value; }

void Logger::setEnabled(bool value) { m_enabled = value; }

void Logger::open(std::string fileName) {
    if (!fileName.empty()) {

        file = new std::ofstream(fileName, std::ios_base::openmode::_S_ate);
        out = file;
    } else {
        out = &std::cerr;
    }
}

Logger &Logger::operator<<(const std::string text) {
    if (m_enabled)
        *out << text;
    return *this;
}

Logger &Logger::operator<<(const int value) {
    if (m_enabled)
        *out << value;
    return *this;
}

Logger &Logger::operator<<(const double value) {
    if (m_enabled)
        *out << value;
    return *this;
}

Logger &Logger::operator<<(const size_t value) {
    if (m_enabled)
        *out << value;
    return *this;
}

Logger &Logger::operator<<(const bool value) {
    if (m_enabled)
        *out << (value ? "True" : "False");
    return *this;
}

bool Logger::enabled(void) { return m_enabled; }

void Logger::close(void) {
    out->flush();

    if (file) {
        file->close();
    }
}

Logger::~Logger() {
    out->flush();

    if (file) {
        file->close();
        delete file;
    }
}
