#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <tiny_gltf.h>
#include "gameplay/transform.h"
#include "render/vulkan/image.h"
#include "render/vulkan/types.h"

namespace render::vulkan {

	class Scene {
	public:
		SceneData scene_data{};

	public:
		Scene(VmaAllocator allocator, size_t buffer_size, SceneData data);
		Scene() = default;
		void destroy();
		void write_to_buffer(size_t offset);
		void write_to_buffer(size_t offset, SceneData data);
		VkDescriptorBufferInfo buffer_info(size_t offset);

	private:
		Buffer mSceneDataBuffer{};
		size_t mBufferSize{0};
		VmaAllocator mAllocator{};
	};

	class GLTFModel {
	public:
		GLTFModel(std::filesystem::path, class VulkanRenderer* renderer);
		~GLTFModel();

	public:
		struct {
			u32 size{0};
			Buffer buffer{};
		} vertices;

		struct {
			u32 count{0};
			Buffer buffer{};
		} indices;

		// A primitive contains the data for a single draw call
		struct Primitive {
			u32 first_index;
			u32 index_count;
			s32 material_index;
		};

		// Contains the node's geometry if there is any
		struct Mesh {
			ArrayList<Primitive> primitives;
		};

		struct Node {
			Node* parent;
			ArrayList<Node*> children;
			Mesh mesh;
			gameplay::Transform transform;

			~Node() {
				for (Node* child : children) {
					delete child;
				}
			}
		};

		struct Material {
			glm::vec4 base_color_factor = glm::vec4(1.f);
			u32 base_color_texture_index;
		};

		struct gltfImage {
			Image image;
			VkDescriptorSet set;
		};

		struct Texture {
			s32 image_index;
		};

		ArrayList<Texture> textures;
		ArrayList<Material> materials;
		ArrayList<gltfImage> images;
		ArrayList<Node*> nodes;
		VkDevice device;
		VkQueue copyQueue;
		class VulkanRenderer* renderer;
        std::string path;

    private:
		void load_images(tinygltf::Model& input);

		void load_textures(tinygltf::Model& input);

		void load_materials(tinygltf::Model& input);
	};
} // namespace render::vulkan
