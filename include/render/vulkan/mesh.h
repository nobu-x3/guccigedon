#pragma once
#include "render/vulkan/types.h"
#include "vk_mem_alloc.h"
#include "render/vulkan/primitives.h"

namespace render::vulkan {

	struct Vertex {
		glm::vec3 position{0, 0, 0};
		glm::vec3 color{0,0,0};
		glm::vec3 normal{0, 0, 0};
		glm::vec2 uv{0, 0};

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

		bool load_primitive(PrimitiveType type);
	};
} // namespace render::vulkan
