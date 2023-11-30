#include "core/logger.h"
#include <ctime>
#include <iomanip>

namespace core {
	std::unique_ptr<Logger> Logger::instance =
		std::unique_ptr<Logger>(new Logger());

	Logger::~Logger() {
		std::unique_lock<std::mutex> lock(mLogMutex);
		bClosing = true;
		mCV.notify_one();
	}

	Logger::Logger() {
		std::time_t t = std::time(nullptr);
		std::tm* tm = std::localtime(&t);
		std::ostringstream oss;
		oss << "logs/log" << std::put_time(tm, "%d-%m-%Y_%H%M%S") << ".log";
		mFileHandle.open(oss.str(), std::ios::out);
		mThread = std::jthread{&Logger::serialize, this};
	}

	Logger& Logger::get() {
		if (instance == nullptr) {
			instance = std::unique_ptr<Logger>(new Logger());
		}
		return *instance;
	}

	void Logger::serialize() {
		while (true) {
			std::unique_lock<std::mutex> lock(mLogMutex);
			mCV.wait(lock,
					 [] { return !instance->bEmpty || instance->bClosing; });
			std::cout << mStream.view() << std::endl;
			if (!bClosing) {
				mFileHandle << mStream.view() << std::endl;
			} else {
				break;
			}
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
