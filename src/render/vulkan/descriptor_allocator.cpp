#include "render/vulkan/descriptor_allocator.h"
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

	DescriptorAllocator&
	DescriptorAllocator::operator=(DescriptorAllocator&& other) noexcept {
		return_to_pool();
		mHandle = other.mHandle;
		mID = other.mID;
		mOwner = other.mOwner;
		other.mOwner = nullptr;
		other.mID = 0;
		other.mHandle = nullptr;
		return *this;
	}

	DescriptorAllocator::~DescriptorAllocator() {
		if (mOwner) {
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
			if (mOwner) {
				mOwner->return_allocator(*this, true);
				*this = mOwner->get_allocator();
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
} // namespace render::vulkan
