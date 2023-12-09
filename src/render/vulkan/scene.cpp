#include "render/vulkan/scene.h"
#include "render/vulkan/types.h"
#include "render/vulkan/renderer.h"
#include <filesystem>

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

    GLTFModel::GLTFModel(std::filesystem::path, VulkanRenderer* renderer){}

    GLTFModel::~GLTFModel(){

    }

	void GLTFModel::load_images(tinygltf::Model& input) {
		images.resize(input.images.size());
		for (int i = 0; i < input.images.size(); ++i) {
            renderer->image_cache().get_image(path + "/" + input.images[i].uri);
		}
	}

	void GLTFModel::load_textures(tinygltf::Model& input) {
		textures.resize(input.textures.size());
		for (int i = 0; i < input.textures.size(); ++i) {
			textures[i].image_index = input.textures[i].source;
		}
	}

	void GLTFModel::load_materials(tinygltf::Model& input) {
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

} // namespace render::vulkan
