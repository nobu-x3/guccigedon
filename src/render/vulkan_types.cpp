#include "vulkan_types.h"
#include "vulkan/vulkan_core.h"

namespace render {
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

	void Buffer::destroy() { vmaDestroyBuffer(mAlloc, handle, memory); }

} // namespace render
