#pragma once

#include <array>
#include <span>
#include <vulkan/vulkan_core.h>
#include "device.h"

namespace render::vulkan {
	class ShaderModule {
	public:
		ShaderModule() = default;
		ShaderModule(VkDevice device, std::string_view filepath);
		ShaderModule(Device& device, std::string_view filepath);
		~ShaderModule();
		ShaderModule(const ShaderModule&);
		ShaderModule& operator=(const ShaderModule&);
		ShaderModule(ShaderModule&&) noexcept;
		ShaderModule& operator=(ShaderModule&&) noexcept;

		inline VkShaderModule handle() const { return mHandle; }
		inline const ArrayList<u32>& code() const { return mCode; }

	private:
		VkDevice mDevice{};
		VkShaderModule mHandle{};
		ArrayList<u32> mCode;
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};
	};

	class ShaderStage {
	public:
		ShaderStage(ShaderModule* mod, VkShaderStageFlagBits stage) :
			mModule(mod), mStageFlags(stage){};

		~ShaderStage() = default;

		inline VkShaderStageFlagBits stage_flags() const { return mStageFlags; }

		inline ShaderModule* module() const { return mModule; }

	private:
		// We do not want to take ownership of this
		ShaderModule* mModule{nullptr};
		VkShaderStageFlagBits mStageFlags{};
	};

	struct ReflectedBinding {
		u32 set, binding;
		VkDescriptorType type;
	};

	struct ReflectionOverride {
		std::string_view name;
		VkDescriptorType override;
	};

	class ShaderSet {
	public:
		/* ~ShaderSet(){ */
		/*     for(auto& set : mSetLayouts){ */
		/*         vkDestroyDescriptorSetLayout(mDevice, set, nullptr); */
		/*     } */
		/*     vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr); */
		/* } */

		inline ShaderSet& add_stage(ShaderStage stage) {
			mStages.push_back(stage);
			return *this;
		}

		inline ShaderSet& add_stage(ShaderModule* mod,
									VkShaderStageFlagBits stage) {
			return add_stage({mod, stage});
		}

		inline const VkPipelineLayout& pipeline_layout() const {
			return mPipelineLayout;
		}

		ArrayList<VkPipelineShaderStageCreateInfo> pipeline_stages();


		void reflect_layout(VkDevice device, std::span<ReflectionOverride> overrides);

	private:
		VkPipelineLayout mPipelineLayout{};
		HashMap<std::string, ReflectedBinding> mBindings{};
		std::array<VkDescriptorSetLayout, 4> mSetLayouts{};
		ArrayList<ShaderStage> mStages;
		VkDevice mDevice{};
	};

	class ShaderCache {
	public:
		ShaderCache() = default;
		ShaderCache(Device& device) : mDevice(device.logical_device()) {}
		~ShaderCache() = default;
		ShaderModule* get_shader(const std::string& path);

	private:
		HashMap<std::string, ShaderModule> mCache;
		VkDevice mDevice{};
	};

} // namespace render::vulkan

template <>
struct std::hash<render::vulkan::ShaderModule> {
	std::size_t operator()(const render::vulkan::ShaderModule& k) const {
		return hash<void*>()(k.handle());
	}
};
