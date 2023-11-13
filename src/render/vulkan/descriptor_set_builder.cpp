#include "render/vulkan/descriptor_set_builder.h"

namespace render::vulkan {
	DescriptorLayoutCache::DescriptorLayoutCache(const Device& device) :
		mDevice(device.logical_device()) {}

	DescriptorLayoutCache::DescriptorLayoutCache(
		DescriptorLayoutCache&& other) noexcept :
		mDevice(other.mDevice),
		mCache(std::move(other.mCache)) {
		other.mDevice = nullptr;
	}

	DescriptorLayoutCache&
	DescriptorLayoutCache::operator=(DescriptorLayoutCache&& other) noexcept {
		mDevice = other.mDevice;
		mCache = std::move(other.mCache);
		other.mDevice = nullptr;
		return *this;
	}

	DescriptorLayoutCache::~DescriptorLayoutCache() {
		if (mDevice) {
			for (auto& [k, v] : mCache) {
				vkDestroyDescriptorSetLayout(mDevice, v, nullptr);
			}
		}
	}

	VkDescriptorSetLayout
	DescriptorLayoutCache::get_layout(VkDescriptorSetLayoutCreateInfo info) {
		DescriptorLayoutInfo layoutinfo;
		layoutinfo.bindings.reserve(info.bindingCount);
		bool sorted = true;
		int32_t lastBinding = -1;
		for (uint32_t i = 0; i < info.bindingCount; i++) {
			layoutinfo.bindings.push_back(info.pBindings[i]);

			// check that the bindings are in strict increasing order
			if (static_cast<int32_t>(info.pBindings[i].binding) > lastBinding) {
				lastBinding = info.pBindings[i].binding;
			} else {
				sorted = false;
			}
		}
		if (!sorted) {
			std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(),
					  [](VkDescriptorSetLayoutBinding& a,
						 VkDescriptorSetLayoutBinding& b) {
						  return a.binding < b.binding;
					  });
		}

		auto it = mCache.find(layoutinfo);
		if (it != mCache.end()) {
			return (*it).second;
		} else {
			VkDescriptorSetLayout layout;
			vkCreateDescriptorSetLayout(mDevice, &info, nullptr, &layout);
			// add to cache
			mCache[layoutinfo] = layout;
			return layout;
		}
	}

	bool
	DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const {
		if (other.bindings.size() != bindings.size()) {
			return false;
		} else {
			// compare each of the bindings is the same. Bindings are sorted so
			// they will match
			for (int i = 0; i < bindings.size(); i++) {
				if (other.bindings[i].binding != bindings[i].binding) {
					return false;
				}
				if (other.bindings[i].descriptorType !=
					bindings[i].descriptorType) {
					return false;
				}
				if (other.bindings[i].descriptorCount !=
					bindings[i].descriptorCount) {
					return false;
				}
				if (other.bindings[i].stageFlags != bindings[i].stageFlags) {
					return false;
				}
			}
			return true;
		}
	}

	size_t DescriptorLayoutInfo::hash() const {
		using std::hash;
		using std::size_t;

		size_t result = hash<size_t>()(bindings.size());

		for (const VkDescriptorSetLayoutBinding& b : bindings) {
			// pack the binding data into a single int64. Not fully correct but
			// its ok
			size_t binding_hash = b.binding | b.descriptorType << 8 |
				b.descriptorCount << 16 | b.stageFlags << 24;
			// shuffle the packed binding data and xor it with the main hash
			result ^= hash<size_t>()(binding_hash);
		}

		return result;
	}

} // namespace render::vulkan
