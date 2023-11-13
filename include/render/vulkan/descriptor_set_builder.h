#pragma once

#include <optional>
#include "core/types.h"
#include "device.h"

namespace render::vulkan {

	struct DescriptorLayoutInfo {
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		bool operator==(const DescriptorLayoutInfo& other) const;

		size_t hash() const;
	};

	class DescriptorLayoutCache {
	public:
		DescriptorLayoutCache() = default;
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
			mCache{};
		VkDevice mDevice{};
	};

	class DescriptorAllocator;
	namespace builder {
		class DescriptorSetBuilder {
		public:
			DescriptorSetBuilder(const Device& device,
								 DescriptorLayoutCache* cache,
								 DescriptorAllocator* allocator);
			~DescriptorSetBuilder() = default;
			DescriptorSetBuilder(const DescriptorSetBuilder&) = delete;
			DescriptorSetBuilder(DescriptorSetBuilder&&) noexcept = delete;
			DescriptorSetBuilder&
			operator=(const DescriptorSetBuilder&) = delete;
			DescriptorSetBuilder& operator=(DescriptorSetBuilder&&) = delete;

			DescriptorSetBuilder&
			add_buffer(u32 binding, VkDescriptorBufferInfo* buffer_info,
						VkDescriptorType type, VkShaderStageFlags stage_flags);

			DescriptorSetBuilder& add_image(u32 binding,
											 VkDescriptorImageInfo* image_info,
											 VkDescriptorType type,
											 VkShaderStageFlags stage_flags);

			std::optional<VkDescriptorSet> build();
			inline VkDescriptorSetLayout layout() const { return mLayout; }

		private:
			VkDevice mDevice;
			DescriptorLayoutCache* mCache;
			DescriptorAllocator* mAllocator;
			VkDescriptorSetLayout mLayout{};
			ArrayList<VkWriteDescriptorSet> mWrites;
			ArrayList<VkDescriptorSetLayoutBinding> mBindings;
		};
	} // namespace builder
} // namespace render::vulkan