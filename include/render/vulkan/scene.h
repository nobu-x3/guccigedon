#pragma once

#include <glm/gtc/type_ptr.hpp>
#include "gameplay/transform.h"
#include "render/vulkan/image.h"
#include "render/vulkan/types.h"

namespace tinygltf {
	class Node;
	class Model;
} // namespace tinygltf

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
		GLTFModel(std::filesystem::path file, Device* device,
				  VulkanRenderer* renderer);
		~GLTFModel();
		// TODO: temp, actually have to implement them...
		GLTFModel() = default;
		GLTFModel(const GLTFModel& other) = delete;
		GLTFModel& operator=(const GLTFModel& other) = delete;
		GLTFModel(GLTFModel&& other) noexcept;
		GLTFModel& operator=(GLTFModel&& other) noexcept;

		void draw(VkCommandBuffer buf, ObjectData* data, FrameData& frame_data,
				  u32 uniform_offset);

		void update(ObjectData* data);

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
			glm::mat4 matrix;
			std::string name;
			bool visible{true};

			~Node() {
				for (Node* child : children) {
					delete child;
				}
			}
		};

		struct gltfImage {
			Image* image;
			VkDescriptorSet set{nullptr};
			VkDescriptorSetLayout layout{nullptr};
		};

		struct Texture {
			s32 image_index;
		};

		ArrayList<Texture> textures;
		ArrayList<Material> materials;
		ArrayList<gltfImage> images;
		ArrayList<Node*> nodes;
		class VulkanRenderer* renderer;
		std::filesystem::path path;

	private:
		void load_images(tinygltf::Model* input);

		void load_textures(tinygltf::Model* input);

		void load_materials(tinygltf::Model* input);

		void load_node(const tinygltf::Node* inputNode,
					   const tinygltf::Model* input, Node* parent,
					   std::vector<uint32_t>& indexBuffer,
					   std::vector<Vertex>& vertexBuffer);

		void draw_node(VkCommandBuffer buf, ObjectData* ssbo, int& ssbo_index,
					   Node* node, FrameData& frame_data, u32 uniform_offset);

		void update_node(ObjectData* ssbo, int& ssbo_index, Node* node);

	private:
		Device* mDevice;
		Buffer mVertexBuffer{};
		Buffer mIndexBuffer{};
		Material mDefaultMaterial{};
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};
	};
} // namespace render::vulkan
