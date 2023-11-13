#pragma once

#include <mutex>
#include <optional>
#include <span>
#include "core/types.h"
#include "vulkan/vulkan_core.h"

namespace render::vulkan {

	class DescriptorAllocatorPool;

	class DescriptorAllocator {
	public:
		friend DescriptorAllocatorPool;
		DescriptorAllocator() = default;
		DescriptorAllocator(DescriptorAllocatorPool* owner, u8 id,
							VkDescriptorPool handle);

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
		//DescriptorAllocator(DescriptorAllocatorPool* owner, u8 id,
		//					VkDescriptorPool handle, bool should_destroy);
		DescriptorAllocatorPool* mOwner{nullptr};
		VkDescriptorPool mHandle{};
		u8 mID{};
		bool mShouldReturn{true};
	};

	struct PoolSize {
		VkDescriptorType type;
		f32 multiplier;
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

	class Device;

	class DescriptorAllocatorPool {
	public:
		DescriptorAllocatorPool() = default;
		DescriptorAllocatorPool(const Device& device, u32 max_frames = 3);
		DescriptorAllocatorPool(const Device& device,
								std::span<PoolSize> pool_sizes,
								u32 max_frames = 3);
		~DescriptorAllocatorPool();
		DescriptorAllocatorPool(const DescriptorAllocatorPool&) = delete;
		DescriptorAllocatorPool&
		operator=(const DescriptorAllocatorPool&) = delete;
		DescriptorAllocatorPool(DescriptorAllocatorPool&&) noexcept;
		DescriptorAllocatorPool& operator=(DescriptorAllocatorPool&&) noexcept;
		DescriptorAllocator&& get_allocator();
		void return_allocator(DescriptorAllocator& allocator, bool isFull);
		VkDescriptorPool create_pool(int count, VkDescriptorPoolCreateFlags);
		void set_pool_size_multiplier(VkDescriptorType, f32);
		inline VkDevice device() const { return mDevice; }

	private:
		struct PoolStorage {
			std::vector<VkDescriptorPool> usableAllocators;
			std::vector<VkDescriptorPool> fullAllocators;
		};

		VkDevice mDevice{};
		PoolSizes mPoolSizes{};
		u32 mFrameIndex{};
		u32 mMaxFrames{};
		std::mutex mPoolMutex{};
		ArrayList<PoolStorage> mDescriptorPools{};
		ArrayList<VkDescriptorPool> mClearedAllocators{};
	};
} // namespace render::vulkan
