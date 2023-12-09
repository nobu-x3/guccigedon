#include "render/vulkan/types.h"
#include <vulkan/vulkan_core.h>

namespace render::vulkan {

	Buffer::Buffer(VmaAllocator alloc, size_t allocation_size,
				   VkBufferUsageFlags usage, VmaMemoryUsage mem_usage) :
		mAlloc(alloc) {
		VkBufferCreateInfo buffer_ci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
									 nullptr, 0, allocation_size, usage};
		VmaAllocationCreateInfo vma_alloc_ci{};
		vma_alloc_ci.usage = mem_usage;
		VK_CHECK(vmaCreateBuffer(mAlloc, &buffer_ci, &vma_alloc_ci, &handle,
								 &memory, nullptr));
	}

	void Buffer::destroy() {
		if (mAlloc && handle && memory) {
			vmaDestroyBuffer(mAlloc, handle, memory);
		}
	}

	Buffer::Buffer(const Buffer& other) :
		mAlloc(other.mAlloc), memory(other.memory), handle(other.handle) {}

	Buffer& Buffer::operator=(const Buffer& other)
    {
        mAlloc = other.mAlloc;
        memory = other.memory;
        handle = other.handle;
        return *this;
    }
    Buffer::	Buffer(Buffer&& other) noexcept{
        mAlloc = other.mAlloc;
        memory = other.memory;
        handle = other.handle;
        other.mAlloc = nullptr;
        other.handle = nullptr;
        other.memory = nullptr;
    }

	Buffer& Buffer::operator=(Buffer&& other) noexcept{
        mAlloc = other.mAlloc;
        memory = other.memory;
        handle = other.handle;
        other.mAlloc = nullptr;
        other.handle = nullptr;
        other.memory = nullptr;
        return *this;
    }
} // namespace render::vulkan
