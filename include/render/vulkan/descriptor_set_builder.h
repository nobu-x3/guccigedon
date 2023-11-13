#pragma once

#include "core/types.h"
#include "device.h"

namespace render::vulkan {

	struct DescriptorLayoutInfo {
		// good idea to turn this into a inlined mCache
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		bool operator==(const DescriptorLayoutInfo& other) const;

		size_t hash() const;
	};

	class DescriptorLayoutCache {
	public:
		DescriptorLayoutCache(const Device& device);
		DescriptorLayoutCache(const DescriptorLayoutCache&) = delete;
		DescriptorLayoutCache& operator=(const DescriptorLayoutCache&) = delete;
		DescriptorLayoutCache(DescriptorLayoutCache&&) noexcept;
		DescriptorLayoutCache& operator=(DescriptorLayoutCache&&) noexcept;
		~DescriptorLayoutCache();
		VkDescriptorSetLayout get_layout(VkDescriptorSetLayoutCreateInfo);

	private:
		struct DescriptorLayoutHash {
			std::size_t operator()(const DescriptorLayoutInfo& k) const {
				return k.hash();
			}
		};

		HashMap<DescriptorLayoutInfo, VkDescriptorSetLayout,
				DescriptorLayoutHash>
			mCache;
		VkDevice mDevice;
	};
} // namespace render::vulkan