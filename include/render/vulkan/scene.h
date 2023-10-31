#pragma once

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
} // namespace render::vulkan
