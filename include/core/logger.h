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
			// This will spawn a new thread. Some compilers reuse it.
			// It's done so that Logger had it's own thread where it does the
			// actual logging without blocking other threads. This needs
			// testing, might be a-ok with context switch.
			// auto f = std::async(std::launch::async, serialize, view);
			std::lock_guard<std::mutex> lock{instance().mLogMutex};
			instance().mStringQueue.emplace_back(str, static_cast<size_t>(len));
			instance().mCV.notify_one();
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
		std::jthread mThread{serialize_q};
		std::list<std::string_view> mStringQueue;
		std::list<std::string_view>::iterator mCurrentLog = mStringQueue.begin();
		std::condition_variable mCV;
		bool mClosing{false};

	private:
		Logger() = default;
		~Logger() {
			{
				std::lock_guard<std::mutex> lock{instance().mLogMutex};
				instance().mClosing = true;
			}
			instance().mCV.notify_one();
		}

		static Logger& instance() {
			static Logger instance;
			return instance;
		}

		static void serialize_q() {
			while (true) {
				std::unique_lock<std::mutex> lock{instance().mLogMutex};
				instance().mCV.wait(lock, [=]{
					return instance().mClosing
						||
						instance().mCurrentLog != instance().mStringQueue.end();
				});
				std::cout << "HELLOO" << std::endl;
				auto& current_log = instance().mCurrentLog;
				auto& closing = instance().mClosing;
				auto& queue = instance().mStringQueue;
				auto& stream = instance().mStream;
				if (closing) {
					while (!queue.empty()) {
						if (current_log == queue.end()) {
							current_log = queue.begin();
						}
						while (current_log != queue.end()) {
							stream << *current_log;
							current_log = queue.erase(current_log);
						}
						std::cout << instance().mStream.str();
						instance().mStream.flush();
						instance().mStream.clear();
						break;
					}
				}
				while (current_log != queue.end()) {
					stream << *current_log;
					current_log = queue.erase(current_log);
				}
				std::cout << instance().mStream.str();
				instance().mStream.flush();
				instance().mStream.clear();
			}
		}

		static void serialize(std::string_view formatted_message) {
			// Same stuff since we only have mutex
			/* std::lock_guard<std::mutex> lock(instance().mLogMutex); */
			std::scoped_lock lock(instance().mLogMutex);
			instance().mStream << formatted_message;
			std::cout << instance().mStream.str();
			instance().mStream.flush();
			instance().mStream.clear();
		}
	};

} // namespace core
