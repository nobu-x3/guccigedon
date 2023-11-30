#include "core/logger.h"
#include <iostream>
#include <mutex>
namespace core {
	Logger* Logger::instance = new Logger();

	Logger::Logger() { mThread = std::jthread{&Logger::serialize, this}; }

	Logger& Logger::get() {
		if (instance == nullptr) {
			instance = new Logger();
		}
		return *instance;
	}

	void Logger::serialize() {
		while (true) {
			std::unique_lock<std::mutex> lock(mLogMutex);
			mCV.wait(lock, [] { return !instance->bEmpty; });
			std::cout << mStream.view() << std::endl;
			std::stringstream stream{};
			std::swap(mStream, stream);
			bEmpty = true;
		}
	}

	void Logger::push(std::string_view view) {
		std::unique_lock<std::mutex> lock(mLogMutex);
		mStream << view;
		bEmpty = false;
		mCV.notify_one();
	}

} // namespace core
