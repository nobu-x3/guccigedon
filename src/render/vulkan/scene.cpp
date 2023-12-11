#include "render/vulkan/scene.h"
#include <filesystem>
#include "render/vulkan/renderer.h"
#include "render/vulkan/types.h"
#define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

namespace render::vulkan {

	Scene::Scene(VmaAllocator allocator, size_t buffer_size, SceneData data) :
		mBufferSize(buffer_size), scene_data(data), mAllocator(allocator) {
		mSceneDataBuffer =
			Buffer(allocator, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				   VMA_MEMORY_USAGE_CPU_TO_GPU);
	}

	void Scene::destroy() { mSceneDataBuffer.destroy(); }

	void Scene::write_to_buffer(size_t offset) {

		char* p_scene_data;
		vmaMapMemory(mAllocator, mSceneDataBuffer.memory,
					 (void**)&p_scene_data);
		p_scene_data += offset;
		memcpy(p_scene_data, &scene_data, sizeof(SceneData));
		vmaUnmapMemory(mAllocator, mSceneDataBuffer.memory);
	}

	void Scene::write_to_buffer(size_t offset, SceneData data) {
		char* p_scene_data;
		vmaMapMemory(mAllocator, mSceneDataBuffer.memory,
					 (void**)&p_scene_data);
		p_scene_data += offset;
		memcpy(p_scene_data, &data, sizeof(SceneData));
		vmaUnmapMemory(mAllocator, mSceneDataBuffer.memory);
	}

	VkDescriptorBufferInfo Scene::buffer_info(size_t offset) {
		return {mSceneDataBuffer.handle, offset, sizeof(SceneData)};
	}

	GLTFModel::GLTFModel(std::filesystem::path file, Device* device,
						 VulkanRenderer* renderer) :
		renderer(renderer),
		path(file.string()), mDevice(device) {
		tinygltf::Model input;
		tinygltf::TinyGLTF context;
		std::string error, warning;
		bool loaded = context.LoadASCIIFromFile(&input, &error, &warning, path);
		ArrayList<u32> index_buffer;
		ArrayList<Vertex> vertex_buffer;
		if (loaded) {
			load_images(&input);
			load_materials(&input);
			load_textures(&input);
			const tinygltf::Scene& scene = input.scenes[0];
			for (int i = 0; i < scene.nodes.size(); ++i) {
				const tinygltf::Node node = input.nodes[scene.nodes[i]];
				load_node(&node, &input, nullptr, index_buffer, vertex_buffer);
			}
		} else {
			throw std::exception();
		}
		const size_t vertex_buf_size = vertex_buffer.size() * sizeof(Vertex);
		const size_t index_buf_size = index_buffer.size() * sizeof(u32);
		Buffer vertex_staging{mDevice->allocator(), vertex_buf_size,
							  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							  VMA_MEMORY_USAGE_CPU_ONLY};
		void* vertex_data;
		vmaMapMemory(mDevice->allocator(), vertex_staging.memory, &vertex_data);
		memcpy(vertex_data, vertex_buffer.data(), vertex_buf_size);
		vmaUnmapMemory(mDevice->allocator(), vertex_staging.memory);
		mVertexBuffer = {mDevice->allocator(), vertex_buf_size,
						 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						 VMA_MEMORY_USAGE_GPU_ONLY};
		Buffer index_staging{mDevice->allocator(), index_buf_size,
							 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							 VMA_MEMORY_USAGE_CPU_ONLY};
		void* index_data;
		vmaMapMemory(mDevice->allocator(), index_staging.memory, &index_data);
		memcpy(index_data, index_buffer.data(), vertex_buf_size);
		vmaUnmapMemory(mDevice->allocator(), index_staging.memory);
		mVertexBuffer = {mDevice->allocator(), index_buf_size,
						 VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						 VMA_MEMORY_USAGE_GPU_ONLY};
		renderer->immediate_submit([=](VkCommandBuffer cmd) {
			VkBufferCopy vertex_copy{0, 0, vertex_buf_size};
			vkCmdCopyBuffer(cmd, vertex_staging.handle, mVertexBuffer.handle, 1,
							&vertex_copy);
			VkBufferCopy index_copy{0, 0, index_buf_size};
			vkCmdCopyBuffer(cmd, index_staging.handle, mVertexBuffer.handle, 1,
							&index_copy);
		});
		vertex_staging.destroy();
		index_staging.destroy();
	}

	GLTFModel::~GLTFModel() {
		mVertexBuffer.destroy();
		mIndexBuffer.destroy();
	}

	void GLTFModel::load_images(tinygltf::Model* in) {
		tinygltf::Model& input = *in;
		images.resize(input.images.size());
		if (!renderer)
			return;
		for (int i = 0; i < input.images.size(); ++i) {
			gltfImage gltfImage{};
			gltfImage.image = renderer->image_cache().get_image(
				path + "/" + input.images[i].uri);
			builder::DescriptorSetBuilder builder{
				renderer->device(), &renderer->descriptor_layout_cache(),
				&renderer->main_descriptor_allocator()};
			VkDescriptorImageInfo image_buf_info{
				gltfImage.image->sampler(), gltfImage.image->view,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
			gltfImage.set = std::move(
				builder
					.add_image(0, &image_buf_info,
							   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
							   VK_SHADER_STAGE_FRAGMENT_BIT)
					.build()
					.value());
		}
	}

	void GLTFModel::load_textures(tinygltf::Model* in) {
		tinygltf::Model& input = *in;
		textures.resize(input.textures.size());
		for (int i = 0; i < input.textures.size(); ++i) {
			textures[i].image_index = input.textures[i].source;
		}
	}

	void GLTFModel::load_materials(tinygltf::Model* in) {
		tinygltf::Model& input = *in;
		materials.resize(input.materials.size());
		for (int i = 0; i < input.materials.size(); ++i) {
			tinygltf::Material mat = input.materials[i];
			if (mat.values.find("baseColorFactor") != mat.values.end()) {
				materials[i].base_color_factor = glm::make_vec4(
					mat.values["baseColorFactor"].ColorFactor().data());
			}
			if (mat.values.find("baseColorTexture") != mat.values.end()) {
				materials[i].base_color_texture_index =
					mat.values["baseColorTexture"].TextureIndex();
			}
		}
	}

	void GLTFModel::load_node(const tinygltf::Node* inNode,
							  const tinygltf::Model* in, Node* parent,
							  std::vector<uint32_t>& indexBuffer,
							  std::vector<Vertex>& vertexBuffer) {
		const tinygltf::Node& inputNode = *inNode;
		const tinygltf::Model& input = *in;
		Node* node = new Node{};
		node->name = inputNode.name;
		node->parent = parent;

		// Get the local node matrix
		// It's either made up from translation, rotation, scale or a 4x4 matrix
		node->matrix = glm::mat4(1.0f);
		if (inputNode.translation.size() == 3) {
			node->matrix = glm::translate(
				node->matrix,
				glm::vec3(glm::make_vec3(inputNode.translation.data())));
		}
		if (inputNode.rotation.size() == 4) {
			glm::quat q = glm::make_quat(inputNode.rotation.data());
			node->matrix *= glm::mat4(q);
		}
		if (inputNode.scale.size() == 3) {
			node->matrix =
				glm::scale(node->matrix,
						   glm::vec3(glm::make_vec3(inputNode.scale.data())));
		}
		if (inputNode.matrix.size() == 16) {
			node->matrix = glm::make_mat4x4(inputNode.matrix.data());
		};

		// Load node's children
		if (inputNode.children.size() > 0) {
			for (size_t i = 0; i < inputNode.children.size(); i++) {
				load_node(&input.nodes[inputNode.children[i]], &input, node,
						  indexBuffer, vertexBuffer);
			}
		}

		// If the node contains mesh data, we load vertices and indices from the
		// buffers In glTF this is done via accessors and buffer views
		if (inputNode.mesh > -1) {
			const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
			// Iterate through all primitives of this node's mesh
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
				uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart =
					static_cast<uint32_t>(vertexBuffer.size());
				uint32_t indexCount = 0;
				// Vertices
				{
					const float* positionBuffer = nullptr;
					const float* normalsBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					const float* tangentsBuffer = nullptr;
					size_t vertexCount = 0;

					// Get buffer data for vertex normals
					if (glTFPrimitive.attributes.find("POSITION") !=
						glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor =
							input.accessors[glTFPrimitive.attributes
												.find("POSITION")
												->second];
						const tinygltf::BufferView& view =
							input.bufferViews[accessor.bufferView];
						positionBuffer = reinterpret_cast<const float*>(&(
							input.buffers[view.buffer]
								.data[accessor.byteOffset + view.byteOffset]));
						vertexCount = accessor.count;
					}
					// Get buffer data for vertex normals
					if (glTFPrimitive.attributes.find("NORMAL") !=
						glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor =
							input.accessors[glTFPrimitive.attributes
												.find("NORMAL")
												->second];
						const tinygltf::BufferView& view =
							input.bufferViews[accessor.bufferView];
						normalsBuffer = reinterpret_cast<const float*>(&(
							input.buffers[view.buffer]
								.data[accessor.byteOffset + view.byteOffset]));
					}
					// Get buffer data for vertex texture coordinates
					// glTF supports multiple sets, we only load the first one
					if (glTFPrimitive.attributes.find("TEXCOORD_0") !=
						glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor =
							input.accessors[glTFPrimitive.attributes
												.find("TEXCOORD_0")
												->second];
						const tinygltf::BufferView& view =
							input.bufferViews[accessor.bufferView];
						texCoordsBuffer = reinterpret_cast<const float*>(&(
							input.buffers[view.buffer]
								.data[accessor.byteOffset + view.byteOffset]));
					}
					// POI: This sample uses normal mapping, so we also need to
					// load the tangents from the glTF file
					if (glTFPrimitive.attributes.find("TANGENT") !=
						glTFPrimitive.attributes.end()) {
						const tinygltf::Accessor& accessor =
							input.accessors[glTFPrimitive.attributes
												.find("TANGENT")
												->second];
						const tinygltf::BufferView& view =
							input.bufferViews[accessor.bufferView];
						tangentsBuffer = reinterpret_cast<const float*>(&(
							input.buffers[view.buffer]
								.data[accessor.byteOffset + view.byteOffset]));
					}

					// Append data to model's vertex buffer
					for (size_t v = 0; v < vertexCount; v++) {
						Vertex vert{};
						vert.position = glm::vec4(
							glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(
							normalsBuffer
								? glm::make_vec3(&normalsBuffer[v * 3])
								: glm::vec3(0.0f)));
						vert.uv = texCoordsBuffer
							? glm::make_vec2(&texCoordsBuffer[v * 2])
							: glm::vec3(0.0f);
						vert.color = glm::vec3(1.0f);
						// vert.tangent = tangentsBuffer
						//	? glm::make_vec4(&tangentsBuffer[v * 4])
						//	: glm::vec4(0.0f);
						vertexBuffer.push_back(vert);
					}
				}
				// Indices
				{
					const tinygltf::Accessor& accessor =
						input.accessors[glTFPrimitive.indices];
					const tinygltf::BufferView& bufferView =
						input.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer =
						input.buffers[bufferView.buffer];

					indexCount += static_cast<uint32_t>(accessor.count);

					// glTF supports different component types of indices
					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
						{
							const uint32_t* buf =
								reinterpret_cast<const uint32_t*>(
									&buffer.data[accessor.byteOffset +
												 bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count;
								 index++) {
								indexBuffer.push_back(buf[index] + vertexStart);
							}
							break;
						}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* buf =
								reinterpret_cast<const uint16_t*>(
									&buffer.data[accessor.byteOffset +
												 bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count;
								 index++) {
								indexBuffer.push_back(buf[index] + vertexStart);
							}
							break;
						}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* buf =
								reinterpret_cast<const uint8_t*>(
									&buffer.data[accessor.byteOffset +
												 bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count;
								 index++) {
								indexBuffer.push_back(buf[index] + vertexStart);
							}
							break;
						}
					default:
						std::cerr << "Index component type "
								  << accessor.componentType << " not supported!"
								  << std::endl;
						return;
					}
				}
				Primitive primitive{};
				primitive.first_index = firstIndex;
				primitive.index_count = indexCount;
				primitive.material_index = glTFPrimitive.material;
				node->mesh.primitives.push_back(primitive);
			}
		}

		if (parent) {
			parent->children.push_back(node);
		} else {
			nodes.push_back(node);
		}
	}

} // namespace render::vulkan
