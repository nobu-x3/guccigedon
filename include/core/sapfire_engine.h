#pragma once

#include <filesystem>
#include "core/types.h"
#include "gameplay/transform.h"
#include "physics/physics_engine.h"
#include "render/vulkan/renderer.h"

namespace core {
	struct Entity {
		s32 physics_index;
		s32 render_index;
		s32 transform_index;
	};

	class Engine {
	public:
		Engine();
		inline Engine(const ArrayList<Entity>& entities,
					  const ArrayList<gameplay::Transform>& transforms) :
			mEntities(entities),
			mTransforms(transforms) {}

		void load_scene(std::filesystem::path scene_path);

		void run();

		inline ArrayList<gameplay::Transform>& transforms() {
			return mTransforms;
		}

		inline u32 entity_count() const { return mEntities.size(); }
    private:
        void load_node(const tinygltf::Node* inNode,
							  const tinygltf::Model* in);

	private:
		ArrayList<Entity> mEntities{};
		ArrayList<gameplay::Transform> mTransforms{};
		std::unique_ptr<render::vulkan::VulkanRenderer> mRenderer;
		std::unique_ptr<physics::Engine> mPhysics;
	};
} // namespace core
