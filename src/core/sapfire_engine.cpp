#include "core/sapfire_engine.h"
#include <glm/gtc/type_ptr.hpp>
#include "assets/scene/gltf_importer.h"
#include "gameplay/transform.h"
#include "tiny_gltf.h"

namespace core {

	Engine::Engine() { mPhysics = std::make_unique<physics::Engine>(this); }

	void Engine::load_scene(std::filesystem::path scene_path) {
		asset::GLTFImporter scene_asset{scene_path};
		for (auto& node_index : scene_asset.scene->nodes) {
			load_node(&scene_asset.input->nodes[node_index], scene_asset.input);
		}
		mRenderer =
			std::make_unique<render::vulkan::VulkanRenderer>(scene_path);
		mPhysics->load_scene(scene_path);
	}

	void Engine::load_node(const tinygltf::Node* inNode,
						   const tinygltf::Model* in) {
		const tinygltf::Node& inputNode = *inNode;
		const tinygltf::Model& input = *in;
		gameplay::Transform transform{};
		if (inputNode.translation.size() == 3) {
			transform.position(glm::make_vec3(inputNode.translation.data()));
		}
		if (inputNode.rotation.size() == 4) {
			transform.rotation(glm::make_quat(inputNode.rotation.data()));
		}
		if (inputNode.scale.size() == 3) {
			transform.scale(glm::make_vec3(inputNode.scale.data()));
		}
		mTransforms.push_back(transform);
		if (inputNode.children.size() > 0) {
			for (const auto& child_index : inputNode.children) {
				load_node(&input.nodes[child_index], in);
			}
		}
	}

	void Engine::run() {
		bool quit = false;
		while (!quit) {
			PollResult poll_result{};
			do {
				poll_result = InputSystem::poll_events();
				if (poll_result.event.type == SDL_QUIT) {
					quit = true;
				}
				// mPhysics->handle_input_event(poll_result);
				mRenderer->handle_input_event(poll_result);
			} while (poll_result.result != 0);
			mPhysics->simulate(0.000016f);
            for(auto& transform : mTransforms){
                transform.calculate_transform();
            }
			mRenderer->draw(mTransforms);
		}
	}
} // namespace core
