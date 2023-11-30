#pragma once

#include <cstdio>
#include <fstream>
#include <format>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

namespace core {
	class Logger {
	public:
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) noexcept = delete;
		Logger(Logger&&) = delete;
		Logger& operator=(Logger&&) noexcept = delete;
		~Logger(); 

		template <typename... Args>
		static void Trace(std::string_view message, Args... args) {
			// @NOTE: I know this is unsafe but you get a whole MB of text
			// space.
			char str[1024];
			int len = std::sprintf(
				str, "%s",
				("[TRACE]\t" +
				 std::vformat(message, std::make_format_args(args...)) + "\n")
					.c_str());
			std::string_view view{str, static_cast<size_t>(len)};
            get().push(view);
		}

		template <typename... Args>
		static void Warning(std::string_view message, Args... args) {
			char str[1024];
			int len = std::sprintf(
				str, "%s",
				("[WARN]\t" +
				 std::vformat(message, std::make_format_args(args...)) + "\n")
					.c_str());
			std::string_view view{str, static_cast<size_t>(len)};
            get().push(view);
		}

		template <typename... Args>
		static void Error(std::string_view message, Args... args) {
			char str[1024];
			int len = std::sprintf(
				str, "%s",
				("[ERROR]\t" +
				 std::vformat(message, std::make_format_args(args...)) + "\n")
					.c_str());
			std::string_view view{str, static_cast<size_t>(len)};
            get().push(view);
		}

		template <typename... Args>
		static void Fatal(std::string_view message, Args... args) {
			char str[1024];
			int len = std::sprintf(
				str, "%s",
				("[FATAL]\t" +
				 std::vformat(message, std::make_format_args(args...)) + "\n")
					.c_str());
			std::string_view view{str, static_cast<size_t>(len)};
            get().push(view);
		}

	private:
        static std::unique_ptr<Logger> instance;
		std::stringstream mStream{};
		std::mutex mLogMutex{};
        std::condition_variable mCV{};
        std::jthread mThread;
        bool bEmpty {false};
		bool bClosing{false};
		std::ofstream mFileHandle;

	private:
		Logger();
        static Logger& get();
        void push(std::string_view);
        void serialize();
	};

} // namespace core
