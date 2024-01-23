#include "assets/scene/gltf_importer.h"
#define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"
namespace asset {

	GLTFImporter::GLTFImporter(std::filesystem::path& scene_path) {
        input = new tinygltf::Model();
		tinygltf::TinyGLTF context;
		std::string error, warning;
		bool loaded = context.LoadASCIIFromFile(input, &error, &warning,
												scene_path.string());
		if (loaded) {
            scene = &input->scenes[0];
		} else {
			throw std::exception();
		}
	}

    GLTFImporter::~GLTFImporter(){
        delete input;
    }
} // namespace asset
