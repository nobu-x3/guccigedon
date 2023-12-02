#include "assets/textures/texture_importer.h"
#include <exception>
#include <filesystem>
#include <system_error>
#include "core/types.h"
#include "ktx.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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

	KTX_Texture::KTX_Texture(std::filesystem::path path) : Texture(path) {
		ktxResult result;
		result = ktxTexture_CreateFromNamedFile(
			path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &mTexture);
		if (result != KTX_SUCCESS) {
			throw std::filesystem::filesystem_error(
				"Failed to find image at given path. See logs.", path,
				std::error_code());
		}
		mWidth = mTexture->baseWidth;
		mHeight = mTexture->baseHeight;
		mMipLevels = mTexture->numLevels;
		// NOTE: i think that's correct?
		mChannels = mTexture->dataSize / (mWidth * mHeight);
		mLayerCount = mTexture->isCubemap ? 6 : 1;
		ktx_uint8_t* mTexture_data = ktxTexture_GetData(mTexture);
		pPixels = mTexture_data;
	}

	KTX_Texture::~KTX_Texture() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (pPixels) {
			ktxTexture_Destroy(mTexture);
		}
	}

	size_t KTX_Texture::offset(int level, int layer, int faceSlice)  {
        size_t offset{0};
		KTX_error_code ret = ktxTexture_GetImageOffset(mTexture, level, layer, faceSlice, &offset);
        if(ret != KTX_SUCCESS){
            throw ret;
        }
        return offset;
    }

	Texture TextureImporter::import(std::filesystem::path path) {
		if (path.extension() == ".ktx") {
			return KTX_Texture(path);
		}

		return STB_Texture(path);
	}

} // namespace asset
