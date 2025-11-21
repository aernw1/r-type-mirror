#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace RType {

    namespace Core {

        enum class LogLevel { Debug = 0, Info = 1, Warning = 2, Error = 3, Critical = 4 };

        class Logger {
        public:
            static void SetLogLevel(LogLevel level) { s_logLevel = level; }

            template <typename... Args>
            static void Debug(const std::string& format, Args&&... args) {
                Log(LogLevel::Debug, format, std::forward<Args>(args)...);
            }

            template <typename... Args>
            static void Info(const std::string& format, Args&&... args) {
                Log(LogLevel::Info, format, std::forward<Args>(args)...);
            }

            template <typename... Args>
            static void Warning(const std::string& format, Args&&... args) {
                Log(LogLevel::Warning, format, std::forward<Args>(args)...);
            }

            template <typename... Args>
            static void Error(const std::string& format, Args&&... args) {
                Log(LogLevel::Error, format, std::forward<Args>(args)...);
            }

            template <typename... Args>
            static void Critical(const std::string& format, Args&&... args) {
                Log(LogLevel::Critical, format, std::forward<Args>(args)...);
            }
        private:
            static inline LogLevel s_logLevel = LogLevel::Debug;

            template <typename... Args>
            static void Log(LogLevel level, const std::string& format, Args&&... args) {
                if (level < s_logLevel) {
                    return;
                }

                std::string message = FormatString(format, std::forward<Args>(args)...);
                std::string levelStr = GetLevelString(level);
                std::string timestamp = GetTimestamp();

                std::ostream& stream = (level >= LogLevel::Error) ? std::cerr : std::cout;
                stream << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;
            }

            static std::string GetLevelString(LogLevel level) {
                switch (level) {
                case LogLevel::Debug:
                    return "DEBUG";
                case LogLevel::Info:
                    return "INFO";
                case LogLevel::Warning:
                    return "WARN";
                case LogLevel::Error:
                    return "ERROR";
                case LogLevel::Critical:
                    return "CRITICAL";
                default:
                    return "UNKNOWN";
                }
            }

            static std::string GetTimestamp() {
                auto now = std::time(nullptr);
                auto tm = *std::localtime(&now);
                std::ostringstream oss;
                oss << std::put_time(&tm, "%H:%M:%S");
                return oss.str();
            }

            template <typename T> static std::string FormatArg(const T& arg) {
                std::ostringstream oss;
                oss << arg;
                return oss.str();
            }

            static std::string FormatString(const std::string& format) { return format; }

            template <typename T, typename... Args>
            static std::string FormatString(const std::string& format, T&& first, Args&&... rest) {
                std::string result = format;
                size_t pos = result.find("{}");
                if (pos != std::string::npos) {
                    result.replace(pos, 2, FormatArg(first));
                }
                return FormatString(result, std::forward<Args>(rest)...);
            }
        };

    }

}
