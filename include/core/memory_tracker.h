#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include "logger.h"

struct MemoryTracker {
	MemoryTracker() {
        std::cout << "Memory tracking."<<std::endl;
    }

	~MemoryTracker() {
		std::cout << "Leaked " << current_usage() << " bytes." << std::endl;
	}
	std::atomic<size_t> allocated_memory{};
	std::atomic<size_t> freed_memory{};
	size_t current_usage() {
		return allocated_memory.load() - freed_memory.load();
	}
    std::mutex mLock{};
    std::unordered_map<void*, size_t> mAddressSizeMap{};

    static void* operator new(size_t size){
        std::cout << "special new" << std::endl;
        return std::malloc(size);
    }

    static void operator delete(void* mem) noexcept {
        std::cout << "special delete" << std::endl;
        std::free(mem);
    }
};


void* operator new(size_t size);
void operator delete(void* memory, size_t size) noexcept;
