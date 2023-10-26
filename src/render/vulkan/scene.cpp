#include "render/vulkan/scene.h"
#include "render/vulkan/vulkan_types.h"

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

	void Scene::write_to_buffer(size_t offset, SceneData data){
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
} // namespace render
