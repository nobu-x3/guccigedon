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

	Buffer& Buffer::operator=(const Buffer& other) {
		mAlloc = other.mAlloc;
		memory = other.memory;
		handle = other.handle;
		return *this;
	}
	Buffer::Buffer(Buffer&& other) noexcept {
		mAlloc = other.mAlloc;
		memory = other.memory;
		handle = other.handle;
		other.mAlloc = nullptr;
		other.handle = nullptr;
		other.memory = nullptr;
	}

	Buffer& Buffer::operator=(Buffer&& other) noexcept {
		mAlloc = other.mAlloc;
		memory = other.memory;
		handle = other.handle;
		other.mAlloc = nullptr;
		other.handle = nullptr;
		other.memory = nullptr;
		return *this;
	}

	VertexInputDescription Vertex::get_description() {
		VertexInputDescription description;
		// we will have just 1 vertex buffer binding, with a per-vertex rate
		VkVertexInputBindingDescription main_binding = {};
		main_binding.binding = 0;
		main_binding.stride = sizeof(Vertex);
		main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		description.bindings.push_back(main_binding);
		// Position will be stored at Location 0
		VkVertexInputAttributeDescription position_attribute = {};
		position_attribute.binding = 0;
		position_attribute.location = 0;
		position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		position_attribute.offset = offsetof(Vertex, position);
		// Normal will be stored at Location 1
		VkVertexInputAttributeDescription normal_attribute = {};
		normal_attribute.binding = 0;
		normal_attribute.location = 1;
		normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		normal_attribute.offset = offsetof(Vertex, normal);
		// Color will be stored at Location 2
		VkVertexInputAttributeDescription color_attribute = {};
		color_attribute.binding = 0;
		color_attribute.location = 2;
		color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		color_attribute.offset = offsetof(Vertex, color);
		VkVertexInputAttributeDescription uv_attribute = {};
		uv_attribute.binding = 0;
		uv_attribute.location = 3;
		uv_attribute.format = VK_FORMAT_R32G32_SFLOAT;
		uv_attribute.offset = offsetof(Vertex, uv);
		description.attributes.push_back(position_attribute);
		description.attributes.push_back(normal_attribute);
		description.attributes.push_back(color_attribute);
		description.attributes.push_back(uv_attribute);
		return description;
	}

	VertexInputDescription SkyboxVertex::get_description() {
		VertexInputDescription description;
		// we will have just 1 vertex buffer binding, with a per-vertex rate
		VkVertexInputBindingDescription main_binding = {};
		main_binding.binding = 0;
		main_binding.stride = sizeof(SkyboxVertex);
		main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		description.bindings.push_back(main_binding);
		// Position will be stored at Location 0
		VkVertexInputAttributeDescription position_attribute = {};
		position_attribute.binding = 0;
		position_attribute.location = 0;
		position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		position_attribute.offset = offsetof(SkyboxVertex, position);
		description.attributes.push_back(position_attribute);
		return description;
	}
} // namespace render::vulkan
