#include "render/vulkan/scene.h"
#include <filesystem>
#include "render/vulkan/pipeline.h"
#include "render/vulkan/renderer.h"
#include "render/vulkan/types.h"
#include "vulkan/vulkan_core.h"
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

	static u32 transform_index{0};
	GLTFModel::GLTFModel(std::filesystem::path file, Device* device,
						 VulkanRenderer* renderer) :
		renderer(renderer),
		path(file), mDevice(device), mLifetime(ObjectLifetime::OWNED) {
		tinygltf::Model input;
		tinygltf::TinyGLTF context;
		std::string error, warning;
		bool loaded =
			context.LoadASCIIFromFile(&input, &error, &warning, path.string());
		ArrayList<u32> index_buffer;
		ArrayList<Vertex> vertex_buffer;
		if (loaded) {
			load_images(&input);
			load_textures(&input);
			load_materials(&input);
			const tinygltf::Scene& scene = input.scenes[0];
			transform_index = 0;
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
		memcpy(index_data, index_buffer.data(), index_buf_size);
		vmaUnmapMemory(mDevice->allocator(), index_staging.memory);
		mIndexBuffer = {mDevice->allocator(), index_buf_size,
						VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
							VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VMA_MEMORY_USAGE_GPU_ONLY};
		renderer->immediate_submit([=, this](VkCommandBuffer cmd) {
			VkBufferCopy vertex_copy{0, 0, vertex_buf_size};
			vkCmdCopyBuffer(cmd, vertex_staging.handle, mVertexBuffer.handle, 1,
							&vertex_copy);
			VkBufferCopy index_copy{0, 0, index_buf_size};
			vkCmdCopyBuffer(cmd, index_staging.handle, mIndexBuffer.handle, 1,
							&index_copy);
		});
		vertex_staging.destroy();
		index_staging.destroy();
	}

	GLTFModel::GLTFModel(const asset::GLTFImporter& scene, Device* device,
						 VulkanRenderer* renderer) :
		renderer(renderer),
		mDevice(device), mLifetime(ObjectLifetime::OWNED) {
		ArrayList<u32> index_buffer;
		ArrayList<Vertex> vertex_buffer;
		load_images(scene.input);
		load_textures(scene.input);
		load_materials(scene.input);
		transform_index = 0;
		for (auto& node_id : scene.scene->nodes) {
			const tinygltf::Node node = scene.input->nodes[node_id];
			load_node(&node, scene.input, nullptr, index_buffer, vertex_buffer);
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
		memcpy(index_data, index_buffer.data(), index_buf_size);
		vmaUnmapMemory(mDevice->allocator(), index_staging.memory);
		mIndexBuffer = {mDevice->allocator(), index_buf_size,
						VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
							VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VMA_MEMORY_USAGE_GPU_ONLY};
		renderer->immediate_submit([=, this](VkCommandBuffer cmd) {
			VkBufferCopy vertex_copy{0, 0, vertex_buf_size};
			vkCmdCopyBuffer(cmd, vertex_staging.handle, mVertexBuffer.handle, 1,
							&vertex_copy);
			VkBufferCopy index_copy{0, 0, index_buf_size};
			vkCmdCopyBuffer(cmd, index_staging.handle, mIndexBuffer.handle, 1,
							&index_copy);
		});
		vertex_staging.destroy();
		index_staging.destroy();
	}

	GLTFModel::GLTFModel(GLTFModel&& other) noexcept :
		renderer(other.renderer), path(std::move(other.path)),
		mDevice(other.mDevice), mLifetime(ObjectLifetime::OWNED) {
		textures = std::move(other.textures);
		materials = std::move(other.materials);
		nodes = std::move(other.nodes);
		images = std::move(other.images);
		mVertexBuffer = std::move(other.mVertexBuffer);
		mIndexBuffer = std::move(other.mIndexBuffer);
		mDefaultMaterial = std::move(other.mDefaultMaterial);
		other.mDevice = nullptr;
		other.mLifetime = ObjectLifetime::TEMP;
		other.renderer = nullptr;
	}

	GLTFModel& GLTFModel::operator=(GLTFModel&& other) noexcept {
		renderer = other.renderer;
		path = std::move(other.path);
		mDevice = other.mDevice;
		mLifetime = ObjectLifetime::OWNED;
		textures = std::move(other.textures);
		materials = std::move(other.materials);
		nodes = std::move(other.nodes);
		images = std::move(other.images);
		mVertexBuffer = std::move(other.mVertexBuffer);
		mIndexBuffer = std::move(other.mIndexBuffer);
		mDefaultMaterial = std::move(other.mDefaultMaterial);
		other.mDevice = nullptr;
		other.mLifetime = ObjectLifetime::TEMP;
		other.renderer = nullptr;
		return *this;
	}

	GLTFModel::~GLTFModel() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		for (auto& material : materials) {
			vkDestroyPipelineLayout(mDevice->logical_device(), material.layout,
									nullptr);
			vkDestroyPipeline(mDevice->logical_device(), material.pipeline,
							  nullptr);
		}
		vkDestroyPipelineLayout(mDevice->logical_device(),
								mDefaultMaterial.layout, nullptr);
		vkDestroyPipeline(mDevice->logical_device(), mDefaultMaterial.pipeline,
						  nullptr);
		mVertexBuffer.destroy();
		mIndexBuffer.destroy();
	}

	void GLTFModel::update(ObjectData* object_ssbo) {
		int ssbo_index{0};
		for (Node* node : nodes) {
			update_node(object_ssbo, ssbo_index, node);
		}
	}

	void GLTFModel::update_node(ObjectData* ssbo, int& ssbo_index, Node* node) {
		if (!node->visible) {
			// TODO: move it way behind camera so that it's culled
			return;
		}
		if (node->mesh.primitives.size() > 0) {
			glm::mat4 node_matrix = node->matrix;
			Node* current_parent = node->parent;
			// Traverse up to find final matrix
			while (current_parent) {
				node_matrix = current_parent->matrix * node_matrix;
				current_parent = current_parent->parent;
			}
			// NOTE: this is potentially wrong, gl_InstanceIndex might be off
			ssbo[ssbo_index].model_matrix = node->matrix;
			ssbo_index++;
		}
		for (Node* child : node->children) {
			update_node(ssbo, ssbo_index, child);
		}
	}

	void GLTFModel::draw(VkCommandBuffer buf, ObjectData* object_ssbo,
						 FrameData& frame_data, u32 uniform_offset) {
		VkDeviceSize offsets[1] = {};
		vkCmdBindVertexBuffers(buf, 0, 1, &mVertexBuffer.handle, offsets);
		vkCmdBindIndexBuffer(buf, mIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
		int ssbo_index{0};
		for (auto& node : nodes) {
			draw_node(buf, object_ssbo, ssbo_index, node, frame_data,
					  uniform_offset);
		}
	}

	void GLTFModel::draw_node(VkCommandBuffer buf, ObjectData* ssbo,
							  int& ssbo_index, Node* node,
							  FrameData& frame_data, u32 uniform_offset) {
		if (!node->visible) {
			// TODO: move it way behind camera so that it's culled
			return;
		}
		if (node->mesh.primitives.size() > 0) {
			Material* current_material{nullptr};
			for (Primitive& primitive : node->mesh.primitives) {
				if (primitive.index_count > 0) {
					if (primitive.material_index >= 0) {
						Material& material =
							materials[primitive.material_index];
						if (current_material != &material) {
							current_material = &material;
							vkCmdBindPipeline(buf,
											  VK_PIPELINE_BIND_POINT_GRAPHICS,
											  material.pipeline);
							vkCmdBindDescriptorSets(
								buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
								material.layout, 0, 1,
								&frame_data.global_descriptor, 1,
								&uniform_offset);
							vkCmdBindDescriptorSets(
								buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
								material.layout, 1, 1,
								&frame_data.object_descriptor, 0, nullptr);
							if (material.texture_set != nullptr) {
								vkCmdBindDescriptorSets(
									buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
									material.layout, 2, 1,
									&material.texture_set, 0, nullptr);
							}
						}
					} else {
						current_material = &mDefaultMaterial;
						vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
										  mDefaultMaterial.pipeline);
						vkCmdBindDescriptorSets(
							buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
							mDefaultMaterial.layout, 0, 1,
							&frame_data.global_descriptor, 1, &uniform_offset);
						vkCmdBindDescriptorSets(
							buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
							mDefaultMaterial.layout, 1, 1,
							&frame_data.object_descriptor, 0, nullptr);
					}
					MeshPushConstant constant;
					constant.id = ssbo_index;
					ssbo_index++;
					vkCmdPushConstants(buf, current_material->layout,
									   VK_SHADER_STAGE_VERTEX_BIT, 0,
									   sizeof(MeshPushConstant), &constant);
					vkCmdDrawIndexed(buf, primitive.index_count, 1,
									 primitive.first_index, 0, 0);
				}
			}
		}
		for (auto& child : node->children) {
			draw_node(buf, ssbo, ssbo_index, child, frame_data, uniform_offset);
		}
	}

	void GLTFModel::load_images(tinygltf::Model* in) {
		tinygltf::Model& input = *in;
		images.resize(input.images.size());
		if (!renderer)
			return;
		for (int i = 0; i < input.images.size(); ++i) {
			gltfImage gltfImage{};
			gltfImage.image = renderer->image_cache().get_image(
				path.parent_path().string() + "/" + input.images[i].uri);
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
			gltfImage.layout = builder.layout();
			images[i] = gltfImage;
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
		{ // default material
			builder::PipelineBuilder builder;
			builder.set_vertex_input_description(Vertex::get_description())
				.add_shader_module(renderer->shader_cache().get_shader(
									   "assets/shaders/"
									   "default_shader.vert.glsl.spv"),
								   ShaderType::VERTEX)
				.add_shader_module(renderer->shader_cache().get_shader(
									   "assets/shaders/"
									   "default_shader.frag.glsl.spv"),
								   ShaderType::FRAGMENT)
				.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false)
				.set_polygon_mode(VK_POLYGON_MODE_FILL)
				.set_cull_mode(VK_CULL_MODE_BACK_BIT,
							   VK_FRONT_FACE_COUNTER_CLOCKWISE)
				.set_multisampling_enabled(false)
				.add_default_color_blend_attachment()
				.set_color_blending_enabled(false)
				.add_push_constant(sizeof(MeshPushConstant),
								   VK_SHADER_STAGE_VERTEX_BIT)
				.add_descriptor_set_layout(renderer->global_descriptor_layout())
				.add_descriptor_set_layout(
					renderer->objects_descriptor_layout())
				.set_depth_testing(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
				.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
				.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
				// .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH)
				.add_viewport(
					{0, 0, static_cast<float>(renderer->window_extent().width),
					 static_cast<float>(renderer->window_extent().height), 0.f,
					 1.f})
				.add_scissor({{0, 0}, renderer->window_extent()});
			mDefaultMaterial.layout =
				builder.build_layout(mDevice->logical_device());
			mDefaultMaterial.pipeline = builder.build_pipeline(
				mDevice->logical_device(), renderer->render_pass());
		}
		tinygltf::Model& input = *in;
		materials.resize(input.materials.size());
		for (int i = 0; i < input.materials.size(); ++i) {
			tinygltf::Material mat = input.materials[i];
			if (mat.values.find("baseColorFactor") != mat.values.end()) {
				materials[i].base_color_factor = glm::make_vec4(
					mat.values["baseColorFactor"].ColorFactor().data());
			}
			builder::PipelineBuilder builder;
			builder.set_vertex_input_description(Vertex::get_description())
				.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false)
				.set_polygon_mode(VK_POLYGON_MODE_FILL)
				.set_cull_mode(VK_CULL_MODE_BACK_BIT,
							   VK_FRONT_FACE_COUNTER_CLOCKWISE)
				.set_multisampling_enabled(false)
				.add_default_color_blend_attachment()
				.set_color_blending_enabled(false)
				.add_push_constant(sizeof(MeshPushConstant),
								   VK_SHADER_STAGE_VERTEX_BIT)
				.add_descriptor_set_layout(renderer->global_descriptor_layout())
				.add_descriptor_set_layout(
					renderer->objects_descriptor_layout())
				.set_depth_testing(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
				.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
				.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
				// .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH)
				.add_viewport(
					{0, 0, static_cast<float>(renderer->window_extent().width),
					 static_cast<float>(renderer->window_extent().height), 0.f,
					 1.f})
				.add_scissor({{0, 0}, renderer->window_extent()});
			if (mat.values.find("baseColorTexture") != mat.values.end()) {
				s32 base_color_texture_index =
					mat.values["baseColorTexture"].TextureIndex();
				materials[i].base_color_texture_index =
					base_color_texture_index;
				materials[i].texture_set =
					images[textures[base_color_texture_index].image_index].set;
				VkDescriptorSetLayout set_layout =
					images[textures[base_color_texture_index].image_index]
						.layout;
				builder
					.add_shader_module(renderer->shader_cache().get_shader(
										   "assets/shaders/"
										   "textured_mesh.vert.glsl.spv"),
									   ShaderType::VERTEX)
					.add_shader_module(renderer->shader_cache().get_shader(
										   "assets/shaders/"
										   "textured_mesh.frag.glsl.spv"),
									   ShaderType::FRAGMENT)
					.add_descriptor_set_layout(set_layout);
			} else {
				builder
					.add_shader_module(renderer->shader_cache().get_shader(
										   "assets/shaders/"
										   "default_shader.vert.glsl.spv"),
									   ShaderType::VERTEX)
					.add_shader_module(renderer->shader_cache().get_shader(
										   "assets/shaders/"
										   "default_shader.frag.glsl.spv"),
									   ShaderType::FRAGMENT);
			}
			materials[i].layout =
				builder.build_layout(mDevice->logical_device());
			materials[i].pipeline = builder.build_pipeline(
				mDevice->logical_device(), renderer->render_pass());
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
		// It's either made up from translation, rotation, scale or a 4x4
		// matrix
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

		// If the node contains mesh data, we load vertices and indices from
		// the buffers In glTF this is done via accessors and buffer views
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
					// glTF supports multiple sets, we only load the first
					// one
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
					// POI: This sample uses normal mapping, so we also need
					// to load the tangents from the glTF file
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
						vert.color = vert.normal;
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
