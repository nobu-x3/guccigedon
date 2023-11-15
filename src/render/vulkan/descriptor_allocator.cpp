#include "render/vulkan/descriptor_allocator.h"
#include <render/vulkan/device.h>
#include <render/vulkan/types.h>
#include "vulkan/vulkan_core.h"

namespace render::vulkan {

	DescriptorAllocator::DescriptorAllocator(
		DescriptorAllocator&& other) noexcept {
		return_to_pool();
		mHandle = other.mHandle;
		mID = other.mID;
		mOwner = other.mOwner;
		other.mOwner = nullptr;
		other.mID = 0;
		other.mHandle = nullptr;
	}

	DescriptorAllocator::DescriptorAllocator(DescriptorAllocatorPool* owner,
											 u8 id, VkDescriptorPool handle) :
		mOwner(owner),
		mID(id), mHandle(handle) {}

	DescriptorAllocator&
	DescriptorAllocator::operator=(DescriptorAllocator&& other) noexcept {
		mHandle = other.mHandle;
		mID = other.mID;
		mOwner = other.mOwner;
		other.mOwner = nullptr;
		other.mID = 0;
		other.mHandle = nullptr;
		return *this;
	}

	DescriptorAllocator::~DescriptorAllocator() {
		if (mOwner && mShouldReturn) {
			mOwner->return_allocator(*this, false);
		}
	}

	std::optional<VkDescriptorSet>
	DescriptorAllocator::allocate(const VkDescriptorSetLayout& layout) {
		VkDescriptorSet set{};
		VkDescriptorSetAllocateInfo alloc_info{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr};
		alloc_info.descriptorPool = mHandle;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &layout;
		VkResult result =
			vkAllocateDescriptorSets(mOwner->device(), &alloc_info, &set);
		if (result != VK_SUCCESS) {
			// probably out of space @TODO: add check for memory errors
			// Currently this will stack overflow if there is a mistake in poll
			// creation
			if (mOwner) {
				mOwner->return_allocator(*this, true);
				*this = mOwner->get_allocator(mID);
				return allocate(layout);
			}
			return {};
		}
		return set;
	}

	void DescriptorAllocator::return_to_pool() {
		assert(mOwner);
		mOwner->return_allocator(*this, false);
		mHandle = nullptr;
		mID = 0;
		mOwner = nullptr;
	}

	DescriptorAllocatorPool::DescriptorAllocatorPool(const Device& device,
													 u32 max_frames) :
		mDevice(device.logical_device()),
		mFrameIndex(0), mMaxFrames(max_frames) {
		for (int i = 0; i < max_frames; ++i) {
			mDescriptorPools.push_back({});
		}
	}

	DescriptorAllocatorPool::DescriptorAllocatorPool(
		const Device& device, std::span<PoolSize> pool_sizes, u32 max_frames) :
		mDevice(device.logical_device()),
		mFrameIndex(0), mMaxFrames(max_frames) {
		mPoolSizes.sizes.reserve(pool_sizes.size());
		for (auto size : pool_sizes) {
			mPoolSizes.sizes.push_back(size);
		}
	}

	DescriptorAllocatorPool::~DescriptorAllocatorPool() {
		std::lock_guard<std::mutex> lock(mPoolMutex);
		if (mDevice) {
			for (auto alloc : mClearedAllocators) {
				vkDestroyDescriptorPool(mDevice, alloc, nullptr);
			}
			for (auto&& storage : mDescriptorPools) {
				for (auto alloc : storage.fullAllocators) {
					vkDestroyDescriptorPool(mDevice, alloc, nullptr);
				}
				for (auto alloc : storage.usableAllocators) {
					vkDestroyDescriptorPool(mDevice, alloc, nullptr);
				}
			}
		}
	}

	DescriptorAllocatorPool::DescriptorAllocatorPool(
		DescriptorAllocatorPool&& other) noexcept :
		mDevice(other.mDevice),
		mPoolSizes(std::move(other.mPoolSizes)), mFrameIndex(other.mFrameIndex),
		mMaxFrames(other.mMaxFrames), mPoolMutex(),
		mDescriptorPools(std::move(other.mDescriptorPools)),
		mClearedAllocators(std::move(other.mClearedAllocators)) {
		other.mDevice = nullptr;
		other.mFrameIndex = 0;
		other.mMaxFrames = 0;
	}

	DescriptorAllocatorPool& DescriptorAllocatorPool::operator=(
		DescriptorAllocatorPool&& other) noexcept {
		mDevice = other.mDevice;
		mPoolSizes = std::move(other.mPoolSizes);
		mFrameIndex = other.mFrameIndex;
		mMaxFrames = other.mMaxFrames;
		mDescriptorPools = std::move(other.mDescriptorPools);
		mClearedAllocators = std::move(other.mClearedAllocators);
		other.mDevice = nullptr;
		other.mFrameIndex = 0;
		other.mMaxFrames = 0;
		return *this;
	}

	DescriptorAllocator& DescriptorAllocatorPool::get_allocator(u32 frame) {
		std::lock_guard<std::mutex> lock(mPoolMutex);
		bool found = false;
		u8 poolIndex = frame;
		VkDescriptorPool allocator;
		if (mClearedAllocators.size() != 0) {
			allocator = mClearedAllocators.back();
			mClearedAllocators.pop_back();
			found = true;
		} else if (mDescriptorPools[poolIndex].usableAllocators.size() != 0) {
			allocator = mDescriptorPools[poolIndex].usableAllocators.back();
			mDescriptorPools[poolIndex].usableAllocators.pop_back();
			found = true;
		}
		if (!found) {
			VkDescriptorPoolCreateFlags flags = 0;
			if (poolIndex == 0) {
				flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			}
			allocator = create_pool(1000, flags);
		}
		DescriptorAllocator alloc = {this, poolIndex, allocator};
		alloc.mShouldReturn = false;
		return std::move(alloc);
	}

	void
	DescriptorAllocatorPool::return_allocator(DescriptorAllocator& allocator,
											  bool isFull) {
		std::lock_guard<std::mutex> lock(mPoolMutex);
		if (isFull) {
			mDescriptorPools[allocator.id()].fullAllocators.push_back(
				allocator.handle());
		} else {
			mDescriptorPools[allocator.id()].usableAllocators.push_back(
				allocator.handle());
		}
		std::cout << "size after returning "
				  << mDescriptorPools[allocator.id()].usableAllocators.size()
				  << std::endl;
	}

	VkDescriptorPool
	DescriptorAllocatorPool::create_pool(int count,
										 VkDescriptorPoolCreateFlags flags) {
		ArrayList<VkDescriptorPoolSize> sizes;
		sizes.reserve(mPoolSizes.sizes.size());
		for (auto size : mPoolSizes.sizes) {
			sizes.emplace_back(size.type,
							   static_cast<u32>(size.multiplier * count));
		}
		VkDescriptorPoolCreateInfo ci{
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr};
		ci.flags = flags;
		ci.maxSets = count;
		ci.poolSizeCount = static_cast<u32>(sizes.size());
		ci.pPoolSizes = sizes.data();
		VkDescriptorPool pool;
		VK_CHECK(vkCreateDescriptorPool(mDevice, &ci, nullptr, &pool));
		return pool;
	}

	void
	DescriptorAllocatorPool::set_pool_size_multiplier(VkDescriptorType type,
													  f32 multiplier) {
		auto it =
			std::find_if(mPoolSizes.sizes.begin(), mPoolSizes.sizes.end(),
						 [=](PoolSize& size) { return size.type == type; });
		if (it == mPoolSizes.sizes.end()) {
			mPoolSizes.sizes.push_back({type, multiplier});
			return;
		}
		it->multiplier = multiplier;
	}
} // namespace render::vulkan
