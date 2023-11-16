#pragma once

#include <cstdio>
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
		~Logger() { std::cout.flush(); }

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
			// This will spawn a new thread. Some compilers reuse it.
			// It's done so that Logger had it's own thread where it does the
			// actual logging without blocking other threads. This needs
			// testing, might be a-ok with context switch.
			//auto f = std::async(std::launch::async, serialize, str);
			std::cout << view;
			std::cout.flush();
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
			auto f = std::async(std::launch::async, serialize, str);
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
			auto f = std::async(std::launch::async, serialize, str);
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
			auto f = std::async(std::launch::async, serialize, str);
		}

	private:
		std::stringstream mStream;
		std::mutex mLogMutex;

	private:
		Logger() = default;
		static Logger& instance() {
			static Logger instance;
			return instance;
		}

		static void serialize(std::string_view formatted_message) {
			// Same stuff since we only have mutex
			/* std::lock_guard<std::mutex> lock(instance().mLogMutex); */
			std::scoped_lock lock(instance().mLogMutex);
			instance().mStream << formatted_message;
			std::cout << instance().mStream.str();
			std::cout.flush();
			instance().mStream.clear();
		}
	};

} // namespace core
