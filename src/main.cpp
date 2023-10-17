#include <iostream>
#include <vector>
#include "core/logger.h"

int main() {
	std::vector<int> vec;
	for (int i = 0; i < 11; ++i) {
		vec.push_back(i);
	}
	for (auto &i : vec) {
        core::Logger::Trace("%d", &i);
	}
	return 0;
}
