#pragma once

#include <mutex>
#include <optional>
#include "core/types.h"
#include "vulkan/vulkan_core.h"

namespace render::vulkan {
	class DescriptorAllocatorPool;

	class DescriptorAllocator {
	public:
		friend DescriptorAllocatorPool;
		DescriptorAllocator() = default;
		DescriptorAllocator(const DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
		DescriptorAllocator(DescriptorAllocator&&) noexcept;
		DescriptorAllocator& operator=(DescriptorAllocator&&) noexcept;
		~DescriptorAllocator();
		std::optional<VkDescriptorSet> allocate(const VkDescriptorSetLayout&);
		void return_to_pool();
		inline VkDescriptorPool handle() const { return mHandle; }
		inline u8 id() const { return mID; }

	private:
		DescriptorAllocatorPool* mOwner{nullptr};
		VkDescriptorPool mHandle{};
		u8 mID{};
	};

	class DescriptorAllocatorPool {

	public:
        DescriptorAllocator&& get_allocator();
        void return_allocator(DescriptorAllocator& allocator, bool isFull);
		inline VkDevice device() const { return mDevice; }

	private:
		struct PoolSize {
			VkDescriptorType type;
			float multiplier;
		};

		struct PoolSizes {
			std::vector<PoolSize> sizes = {
				{VK_DESCRIPTOR_TYPE_SAMPLER, 1.f},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f},
				{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f},
				{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f},
				{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f},
				{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1.f}};
		};

		struct PoolStorage {
			std::vector<DescriptorAllocator> _usableAllocators;
			std::vector<DescriptorAllocator> _fullAllocators;
		};

		VkDevice mDevice;
		PoolSizes mPoolSizes;
		u32 mFrameIndex;
		u32 mMaxFrames;
		std::mutex mPoolMutex;
		ArrayList<PoolStorage> mDescriptorPools;
		ArrayList<DescriptorAllocator> mClearedAllocators;
	};
} // namespace render::vulkan
