#pragma once
#include "vk_mem_alloc.h"
#include "vulkan_builders.h"
#include "vulkan_types.h"

namespace render {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal;

		static vkbuild::VertexInputDescription get_description();
	};

	class Mesh {
	public:
		ArrayList<Vertex> vertices{};

		Buffer buffer{};

        glm::mat4 transform{};

	public:
		void deinit(VmaAllocator alloc);

		Mesh& set_vertices(ArrayList<Vertex>& data);


		bool load_from_obj(const char* path);
	};
} // namespace render
