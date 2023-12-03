#pragma once
#include "render/vulkan/types.h"
#include "vk_mem_alloc.h"

namespace render::vulkan {

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 uv;

		static VertexInputDescription get_description();
	};

	struct SkyboxVertex {
		glm::vec3 position;

		static VertexInputDescription get_description();
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
} // namespace render::vulkan
