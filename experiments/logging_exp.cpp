#include <vector>
#include "core/logger.h"

int main() {
	std::vector<std::jthread> threads;
	for (int i = 0; i < 10; ++i) {
		threads.emplace_back([i]() {
			while (true) {
				core::Logger::Trace("Hello from {}", i);
			}
		});
	}

	return 0;
}
