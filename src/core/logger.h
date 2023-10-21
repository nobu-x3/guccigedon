#pragma once

#include <cstdio>
namespace core {
	class Logger {

	public:
		template <typename... Args>
		static void Trace(const char* message, Args... args) {
			printf("[TRACE]\t");
			printf(message, args...);
			printf("\n");
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
	};
} // namespace core
