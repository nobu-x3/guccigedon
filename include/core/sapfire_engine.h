#pragma once

#include "core/types.h"
#include "gameplay/transform.h"
#include "render/vulkan/renderer.h"

namespace core {
	struct Entity {
		u32 physics_index;
		u32 render_index;
		gameplay::Transform transform;
	};

	class Engine {
	public:
		void load_scene(std::filesystem::path scene_path);
		void run();

	private:
		ArrayList<Entity> mEntities{};
		std::unique_ptr<render::vulkan::VulkanRenderer> mRenderer;
	};
} // namespace core
