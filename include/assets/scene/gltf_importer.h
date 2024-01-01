#pragma once

#include <filesystem>
namespace tinygltf {
	class Scene;
	class Model;
    class Node;
} // namespace tinygltf

namespace asset {
	class GLTFImporter {
	public:
		GLTFImporter(std::filesystem::path& scene_path);
        ~GLTFImporter();
		tinygltf::Scene* scene;
		tinygltf::Model* input;
	};
} // namespace asset
