#include "render/vulkan/shader.h"
#include <fstream>
#undef SF_SPV_REFLECT
#ifdef SF_SPV_REFLECT
#include <spirv_reflect.h>
#endif
#include "render/vulkan/builders.h"
#include "render/vulkan/types.h"

namespace render::vulkan {
	ShaderModule::ShaderModule(VkDevice device, std::string_view filepath) :
		mDevice(device), mLifetime(ObjectLifetime::OWNED) {
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
		mCode = std::move(buffer);
	}

	ShaderModule::ShaderModule(Device& device, std::string_view filepath) :
		ShaderModule(device.logical_device(), filepath) {}

	ShaderModule::ShaderModule(const ShaderModule& other) :
		mDevice(other.mDevice), mHandle(other.mHandle),
		mLifetime(ObjectLifetime::TEMP), mCode(other.mCode) {}

	ShaderModule& ShaderModule::operator=(const ShaderModule& other) {
		mHandle = other.mHandle;
		mDevice = other.mDevice;
		mLifetime = ObjectLifetime::TEMP;
		mCode = std::move(other.mCode);
		return *this;
	}

	ShaderModule::~ShaderModule() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (mDevice && mHandle) {
			vkDestroyShaderModule(mDevice, mHandle, nullptr);
		}
	}

	ShaderModule::ShaderModule(ShaderModule&& other) noexcept :
		mDevice(other.mDevice), mHandle(other.mHandle),
		mLifetime(ObjectLifetime::OWNED), mCode(std::move(other.mCode)) {
		other.mLifetime = ObjectLifetime::TEMP;
		other.mDevice = nullptr;
		other.mHandle = nullptr;
	}

	ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept {
		mHandle = other.mHandle;
		mDevice = other.mDevice;
		mLifetime = ObjectLifetime::OWNED;
		mCode = std::move(other.mCode);
		other.mDevice = nullptr;
		other.mHandle = nullptr;
		other.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	ShaderModule* ShaderCache::get_shader(const std::string& path) {
		if (!mCache.contains(path)) {
			mCache[path] = {mDevice, path};
		}
		return &mCache[path];
	}

	ArrayList<VkPipelineShaderStageCreateInfo> ShaderSet::pipeline_stages() {
		ArrayList<VkPipelineShaderStageCreateInfo> stages;
		stages.reserve(2);
		for (auto& s : mStages) {
			stages.push_back(builder::pipeline_shader_stage_ci(
				s.module()->handle(), s.stage_flags()));
		}
		return stages;
	}

	struct DescriptorSetLayoutData {
		u32 set_number;
		VkDescriptorSetLayoutCreateInfo ci;
		ArrayList<VkDescriptorSetLayoutBinding> bindings;
	};

	#ifdef SF_SPV_REFLECT
	void ShaderSet::reflect_layout(VkDevice device, std::span<ReflectionOverride> overrides) {
		std::vector<DescriptorSetLayoutData> set_layouts;
		std::vector<VkPushConstantRange> constant_ranges;
		for (auto& s : mStages) {
			SpvReflectShaderModule spvmodule;
			SpvReflectResult result = spvReflectCreateShaderModule(
				s.module()->code().size() * sizeof(uint32_t),
				s.module()->code().data(), &spvmodule);
			uint32_t count = 0;
			result =
				spvReflectEnumerateDescriptorSets(&spvmodule, &count, NULL);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			std::vector<SpvReflectDescriptorSet*> sets(count);
			result = spvReflectEnumerateDescriptorSets(&spvmodule, &count,
													   sets.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			for (size_t i_set = 0; i_set < sets.size(); ++i_set) {
				const SpvReflectDescriptorSet& refl_set = *(sets[i_set]);
				DescriptorSetLayoutData layout = {};
				layout.bindings.resize(refl_set.binding_count);
				for (uint32_t i_binding = 0; i_binding < refl_set.binding_count;
					 ++i_binding) {
					const SpvReflectDescriptorBinding& refl_binding =
						*(refl_set.bindings[i_binding]);
					VkDescriptorSetLayoutBinding& layout_binding =
						layout.bindings[i_binding];
					layout_binding.binding = refl_binding.binding;
					layout_binding.descriptorType =
						static_cast<VkDescriptorType>(
							refl_binding.descriptor_type);
					for (int ov = 0; ov < overrides.size(); ov++) {
						if (strcmp(refl_binding.name,
								   overrides[ov].name.data()) == 0) {
							layout_binding.descriptorType =
								overrides[ov].override;
						}
					}
					layout_binding.descriptorCount = 1;
					for (uint32_t i_dim = 0;
						 i_dim < refl_binding.array.dims_count; ++i_dim) {
						layout_binding.descriptorCount *=
							refl_binding.array.dims[i_dim];
					}
					layout_binding.stageFlags =
						static_cast<VkShaderStageFlagBits>(
							spvmodule.shader_stage);
					ReflectedBinding reflected;
					reflected.binding = layout_binding.binding;
					reflected.set = refl_set.set;
					reflected.type = layout_binding.descriptorType;
					mBindings[refl_binding.name] = reflected;
				}
				layout.set_number = refl_set.set;
				layout.ci.sType =
					VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layout.ci.bindingCount = refl_set.binding_count;
				layout.ci.pBindings = layout.bindings.data();
				set_layouts.push_back(layout);
			}
			// pushconstants
			result =
				spvReflectEnumeratePushConstantBlocks(&spvmodule, &count, NULL);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			std::vector<SpvReflectBlockVariable*> pconstants(count);
			result = spvReflectEnumeratePushConstantBlocks(&spvmodule, &count,
														   pconstants.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);
			if (count > 0) {
				VkPushConstantRange pcs{};
				pcs.offset = pconstants[0]->offset;
				pcs.size = pconstants[0]->size;
				pcs.stageFlags = s.stage_flags();
				constant_ranges.push_back(pcs);
			}
		}
		std::array<DescriptorSetLayoutData, 4> merged_layouts;
		for (int i = 0; i < 4; i++) {
			DescriptorSetLayoutData& ly = merged_layouts[i];
			ly.set_number = i;
			ly.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			std::unordered_map<int, VkDescriptorSetLayoutBinding> binds;
			for (auto& s : set_layouts) {
				if (s.set_number == i) {
					for (auto& b : s.bindings) {
						auto it = binds.find(b.binding);
						if (it == binds.end()) {
							binds[b.binding] = b;
							// ly.bindings.push_back(b);
						} else {
							// merge flags
							binds[b.binding].stageFlags |= b.stageFlags;
						}
					}
				}
			}
			for (auto [k, v] : binds) {
				ly.bindings.push_back(v);
			}
			// sort the bindings, for hash purposes
			std::sort(ly.bindings.begin(), ly.bindings.end(),
					  [](VkDescriptorSetLayoutBinding& a,
						 VkDescriptorSetLayoutBinding& b) {
						  return a.binding < b.binding;
					  });
			ly.ci.bindingCount = (uint32_t)ly.bindings.size();
			ly.ci.pBindings = ly.bindings.data();
			ly.ci.flags = 0;
			ly.ci.pNext = 0;
			if (ly.ci.bindingCount > 0) {
				vkCreateDescriptorSetLayout(device, &ly.ci, nullptr,
											&mSetLayouts[i]);
			} else {
				mSetLayouts[i] = VK_NULL_HANDLE;
			}
		}
		// we start from just the default empty pipeline layout info
		VkPipelineLayoutCreateInfo mesh_pipeline_layout_info =
			builder::pipeline_layout_ci();
		mesh_pipeline_layout_info.pPushConstantRanges = constant_ranges.data();
		mesh_pipeline_layout_info.pushConstantRangeCount =
			(uint32_t)constant_ranges.size();
		std::array<VkDescriptorSetLayout, 4> compactedLayouts;
		int s = 0;
		for (int i = 0; i < 4; i++) {
			if (mSetLayouts[i] != VK_NULL_HANDLE) {
				compactedLayouts[s] = mSetLayouts[i];
				s++;
			}
		}
		mesh_pipeline_layout_info.setLayoutCount = s;
		mesh_pipeline_layout_info.pSetLayouts = compactedLayouts.data();
		vkCreatePipelineLayout(device, &mesh_pipeline_layout_info, nullptr,
							   &mPipelineLayout);
	}
	#endif
} // namespace render::vulkan
