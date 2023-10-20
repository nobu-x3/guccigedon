#include "vulkan_mesh.h"
#include "vk_mem_alloc.h"
namespace render {

	vkbuild::VertexInputDescription Vertex::get_description() {
		vkbuild::VertexInputDescription description;
		// we will have just 1 vertex buffer binding, with a per-vertex rate
		VkVertexInputBindingDescription mainBinding = {};
		mainBinding.binding = 0;
		mainBinding.stride = sizeof(Vertex);
		mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		description.bindings.push_back(mainBinding);
		// Position will be stored at Location 0
		VkVertexInputAttributeDescription positionAttribute = {};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		positionAttribute.offset = offsetof(Vertex, position);
		// Normal will be stored at Location 1
		VkVertexInputAttributeDescription normalAttribute = {};
		normalAttribute.binding = 0;
		normalAttribute.location = 1;
		normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		normalAttribute.offset = offsetof(Vertex, normal);
		// Color will be stored at Location 2
		VkVertexInputAttributeDescription colorAttribute = {};
		colorAttribute.binding = 0;
		colorAttribute.location = 2;
		colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		colorAttribute.offset = offsetof(Vertex, color);
		description.attributes.push_back(positionAttribute);
		description.attributes.push_back(normalAttribute);
		description.attributes.push_back(colorAttribute);
		return description;
	}

	void Mesh::deinit(VmaAllocator alloc) {
		vmaDestroyBuffer(alloc, buffer.handle, buffer.memory);
		vmaFreeMemory(alloc, buffer.memory);
	}

	Mesh& Mesh::set_vertices(ArrayList<Vertex>& data) {
		vertices = std::move(data);
		return *this;
	}

	Mesh& Mesh::upload_mesh(VmaAllocator alloc) {
		VkBufferCreateInfo buf_ci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
								  nullptr};
		buf_ci.size = vertices.size() * sizeof(Vertex);
		buf_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		VK_CHECK(vmaCreateBuffer(alloc, &buf_ci, &alloc_info, &buffer.handle,
								 &buffer.memory, nullptr));
		void* data;
		vmaMapMemory(alloc, buffer.memory, &data);
		memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
		vmaUnmapMemory(alloc, buffer.memory);
		return *this;
	}
} // namespace render
