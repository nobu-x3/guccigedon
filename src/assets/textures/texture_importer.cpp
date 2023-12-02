#include "assets/textures/texture_importer.h"
#include <exception>
#include <filesystem>
#include <system_error>
#include "core/types.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "core/logger.h"

namespace asset {

	Texture::Texture(const Texture& other) {
		pPixels = other.pPixels;
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mChannels = other.mChannels;
		mLifetime = ObjectLifetime::TEMP;
	}

	Texture& Texture::operator=(const Texture& other) {
		pPixels = other.pPixels;
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mChannels = other.mChannels;
		mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Texture::Texture(Texture&& other) noexcept {
		pPixels = other.pPixels;
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mChannels = other.mChannels;
		mLifetime = ObjectLifetime::OWNED;
		other.pPixels = nullptr;
		other.mWidth = 0;
		other.mHeight = 0;
		other.mChannels = 0;
		other.mLifetime = ObjectLifetime::TEMP;
	}

	Texture& Texture::operator=(Texture&& other) noexcept {
		pPixels = other.pPixels;
		mWidth = other.mWidth;
		mHeight = other.mHeight;
		mChannels = other.mChannels;
		mLifetime = ObjectLifetime::OWNED;
		other.pPixels = nullptr;
		other.mWidth = 0;
		other.mHeight = 0;
		other.mChannels = 0;
		other.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	STB_Texture::STB_Texture(std::filesystem::path path) : Texture(path) {
		stbi_uc* pixels = stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels,
									STBI_rgb_alpha);
		if (!pixels) {
			// @TODO: load default texture instead of exiting
			throw std::filesystem::filesystem_error(
				"Failed to find image at given path. See logs.", path,
				std::error_code());
		}
		pPixels = pixels;
	}

	STB_Texture::~STB_Texture() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (pPixels) {
			stbi_image_free(pPixels);
		}
	}

	Texture TextureImporter::import(std::filesystem::path path) {
		if (path.extension() == ".ktx") {
		}

		return STB_Texture(path);
	}

} // namespace asset
