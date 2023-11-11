#include <iostream>
#include <memory>
#include <mutex>
#include <vector>
#include "core/core.h"
#include "core/memory_tracker.h"
#include "render/vulkan/renderer.h"

MemoryTracker tracker{};

void* operator new(size_t size) {
	void* mem = std::malloc(size);
	if (mem) {
		size_t allocated = tracker.allocated_memory.load();
		tracker.allocated_memory.store(allocated + size);
        tracker.mAddressSizeMap[mem] = size;
	} else {
		core::Logger::Error("Failed to allocate memory");
	}
	return mem;
}

void operator delete(void* memory) noexcept {
	std::free(memory);
	size_t freed = tracker.freed_memory.load();
	tracker.freed_memory.store(freed);
}

int main() {
	render::vulkan::VulkanRenderer renderer{};
	renderer.run();
	auto ptr = std::make_unique<int>(0);
	return 0;
}
