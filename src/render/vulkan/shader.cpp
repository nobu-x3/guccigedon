#include "render/vulkan/shader.h"
#include <fstream>
#include <spirv_reflect.h>
#include "render/vulkan/builders.h"

namespace render::vulkan {
	ShaderModule::ShaderModule(VkDevice device, std::string_view filepath) :
		mDevice(device) {
		// open the file. With cursor at the end
		std::ifstream file(filepath.data(), std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			core::Logger::Error("Failed to open shader code at path %s",
								filepath);
			throw std::exception();
		}
		// find what the size of the file is by looking up the location of
		// the cursor because the cursor is at the end, it gives the size
		// directly in bytes
		size_t file_size = (size_t)file.tellg();
		// spirv expects the buffer to be on uint32, so make sure to reserve
		// a int vector big enough for the entire file
		ArrayList<u32> buffer(file_size / sizeof(u32));
		// put file cursor at beggining
		file.seekg(0);
		// load the entire file into the buffer
		file.read((char*)buffer.data(), file_size);
		// now that the file is loaded into the buffer, we can close it
		file.close();
		// create a new shader module, using the buffer we loaded
		// codeSize has to be in bytes, so multply the ints in the buffer by
		// size of int to know the real size of the buffer
		VkShaderModuleCreateInfo createInfo = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0,
			buffer.size() * sizeof(u32), buffer.data()};
		// check that the creation goes well.
		VkResult res =
			vkCreateShaderModule(mDevice, &createInfo, nullptr, &mHandle);
		if (res != VK_SUCCESS) {
			core::Logger::Error("Failed to create shader module. {}", (int)res);
			throw std::exception();
		}
	}

	ShaderModule::ShaderModule(Device& device, std::string_view filepath) :
		ShaderModule(device.logical_device(), filepath) {}

	ShaderModule::~ShaderModule() {
		if (mDevice && mHandle) {
			vkDestroyShaderModule(mDevice, mHandle, nullptr);
		}
	}

	ShaderModule::ShaderModule(ShaderModule&& other) noexcept :
		mDevice(other.mDevice), mHandle(other.mHandle) {
		other.mDevice = nullptr;
		other.mHandle = nullptr;
	}

	ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
		mHandle = other.mHandle;
		mDevice = other.mDevice;
		other.mDevice = nullptr;
		other.mHandle = nullptr;
		return *this;
	}

	ShaderModule* ShaderCache::get_shader(const std::string& path) {
		if (!mCache.contains(path)) {
			mCache[path] = {mDevice, path};
		}
		return &mCache[path];
	}

	ArrayList<VkPipelineShaderStageCreateInfo> ShaderSet::pipeline_stages() {
		ArrayList<VkPipelineShaderStageCreateInfo> stages(2);
		for (auto& s : mStages) {
			stages.push_back(vkbuild::pipeline_shader_stage_ci(
				s.module()->handle(), s.stage_flags()));
		}
		return stages;
	}

	struct DescriptorSetLayoutData {
		u32 set_number;
		VkDescriptorSetLayoutCreateInfo ci;
		ArrayList<VkDescriptorSetLayoutBinding> bindings;
	};

	void ShaderSet::reflect_layout(VkDevice device,
								   std::span<ReflectionOverride> overrides) {
		ArrayList<DescriptorSetLayoutData> set_layouts(4);
		ArrayList<VkPushConstantRange> constant_ranges(4);
		for (auto& s : mStages) {
			SpvReflectShaderModule spv_module;
			SpvReflectResult result = spvReflectCreateShaderModule(
				s.module()->code().size() * sizeof(u32),
				s.module()->code().data(), &spv_module);
			u32 count = 0;
			result =
				spvReflectEnumerateDescriptorSets(&spv_module, &count, nullptr);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			ArrayList<SpvReflectDescriptorSet*> sets(count);
			result = spvReflectEnumerateDescriptorSets(&spv_module, &count,
													   sets.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			// ok not a fan of but it is what it is
			for (int i_set = 0; i_set < sets.size(); ++i_set) {
				const SpvReflectDescriptorSet& r_set = *(sets[i_set]);
				DescriptorSetLayoutData layout{};
				layout.bindings.resize(r_set.binding_count);
				for (int i_binding = 0; i_binding < r_set.binding_count;
					 ++i_binding) {
					const SpvReflectDescriptorBinding& r_binding =
						*(r_set.bindings[i_binding]);
					VkDescriptorSetLayoutBinding& layout_binding =
						layout.bindings[i_binding];
					layout_binding.binding = r_binding.binding;
					layout_binding.descriptorType =
						static_cast<VkDescriptorType>(
							r_binding.descriptor_type);
					for (auto ov : overrides) {
						if (strcmp(r_binding.name, ov.name.data()) == 0)
							layout_binding.descriptorType = ov.override;
					}
					layout_binding.descriptorCount = 1;
					for (auto dim : r_binding.array.dims) {
						layout_binding.descriptorCount *= dim;
					}
					layout_binding.stageFlags =
						static_cast<VkShaderStageFlagBits>(
							spv_module.shader_stage);
					ReflectedBinding reflected_binding;
					reflected_binding.set = r_set.set;
					reflected_binding.binding = r_binding.binding;
					reflected_binding.type = layout_binding.descriptorType;
					mBindings[r_binding.name] = reflected_binding;
				}
				layout.set_number = r_set.set;
				layout.ci.sType =
					VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layout.ci.bindingCount = r_set.binding_count;
				layout.ci.pBindings = layout.bindings.data();
				set_layouts.push_back(layout);
			}
			result = spvReflectEnumeratePushConstantBlocks(&spv_module, &count,
														   nullptr);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			ArrayList<SpvReflectBlockVariable*> push_constants(count);
			result = spvReflectEnumeratePushConstantBlocks(
				&spv_module, &count, push_constants.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			if (count > 0) {
				VkPushConstantRange pcs{};
				pcs.offset = push_constants[0]->offset;
				pcs.size = push_constants[0]->size;
				pcs.stageFlags = s.stage_flags();
				constant_ranges.push_back(pcs);
			}
		}
		// merge layouts
		std::array<DescriptorSetLayoutData, 4> merged_layouts;
		for (int i = 0; i < 4; ++i) {
			DescriptorSetLayoutData& layout = merged_layouts[i];
			layout.set_number = i;
			layout.ci.sType =
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			HashMap<int, VkDescriptorSetLayoutBinding> bindings;
			for (auto& s : set_layouts) {

				if (s.set_number == i) {
					for (auto& b : s.bindings) {
						if (bindings.contains(b.binding)) {
							bindings[b.binding].stageFlags |= b.stageFlags;
						} else {
							bindings[b.binding] = b;
						}
					}
				}
			}
			for (auto [k, v] : bindings) {
				layout.bindings.push_back(v);
			}
			// sort if wanna do hashes
			std::sort(layout.bindings.begin(), layout.bindings.end(),
					  [](VkDescriptorSetLayoutBinding& a,
						 VkDescriptorSetLayoutBinding& b) {
						  return a.binding < b.binding;
					  });
			layout.ci.bindingCount = (u32)layout.bindings.size();
			layout.ci.pBindings = layout.bindings.data();
			layout.ci.flags = 0;
			layout.ci.pNext = nullptr;
			if (layout.ci.bindingCount > 0) {
				vkCreateDescriptorSetLayout(device, &layout.ci, nullptr,
											&mSetLayouts[i]);
			} else {
				mSetLayouts[i] = nullptr;
			}
		}
		VkPipelineLayoutCreateInfo pipeline_layout_ci =
			vkbuild::pipeline_layout_ci();
		pipeline_layout_ci.pushConstantRangeCount =
			static_cast<u32>(constant_ranges.size());
		pipeline_layout_ci.pPushConstantRanges = constant_ranges.data();
		std::array<VkDescriptorSetLayout, 4> layouts{};
		int s = 0;
		for (int i = 0; i < 4; ++i) {
			if (mSetLayouts[i] != nullptr) {
				layouts[s] = mSetLayouts[i];
				s++;
			}
		}
		pipeline_layout_ci.setLayoutCount = s;
		pipeline_layout_ci.pSetLayouts = layouts.data();
		vkCreatePipelineLayout(device, &pipeline_layout_ci, nullptr,
							   &mPipelineLayout);
	}
} // namespace render::vulkan
