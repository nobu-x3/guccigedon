#pragma once
#include "render/vulkan/primitives.h"
#include "render/vulkan/types.h"
#include "vk_mem_alloc.h"

namespace render::vulkan {

	class Mesh {
	public:
		ArrayList<Vertex> vertices{};

		Buffer buffer{};

		glm::mat4 transform{};

	public:
		void deinit(VmaAllocator alloc);

		Mesh& set_vertices(ArrayList<Vertex>& data);

		bool load_from_obj(const char* path);

		bool load_primitive(PrimitiveType type);
	};
} // namespace render::vulkan
