#pragma once

#include <cstdio>
#include <format>
#include <iostream>
#include <memory>
#include <sstream>
#include <string_view>

namespace core {
	class Logger {
	public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) noexcept = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(Logger&&) noexcept = delete;

		template <typename... Args>
		static void Trace(std::string_view message, Args... args) {
			instance().mStream << "[TRACE]\t"
					<< std::vformat(message, std::make_format_args(args...))
					<< std::endl;
		}

		template <typename... Args>
		static void Warning(const char* message, Args... args) {
			printf("[WARN]\t");
			printf(message, args...);
			printf("\n");
		}

		template <typename... Args>
		static void Error(const char* message, Args... args) {
			printf("[ERROR]\t");
			printf(message, args...);
			printf("\n");
		}

		template <typename... Args>
		static void Fatal(const char* message, Args... args) {
			printf("[FATAL]\t");
			printf(message, args...);
			printf("\n");
		}

	private:
		std::stringstream mStream;

    private:
        Logger() = default;
        static Logger& instance(){
            static Logger instance;
            return instance;
        }
	};

} // namespace core
