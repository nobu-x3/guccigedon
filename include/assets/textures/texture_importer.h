#pragma once

#include <concepts>
#include <filesystem>
#include "core/types.h"
#include "ktx.h"

namespace asset {
	class TextureImporter;

	enum class TextureType { color = 0, cubemap = 1 };

	// Should never be constructed manually,
	// use TextureImporter::import, otherwise will crash or leak.
	class Texture {
	public:
		friend TextureImporter;
		virtual ~Texture() {}
		Texture(const Texture&);
		virtual Texture& operator=(const Texture&);
		Texture(Texture&&) noexcept;
		virtual Texture& operator=(Texture&&) noexcept;
		inline int width() const { return mWidth; }
		inline int height() const { return mHeight; }
		inline int channels() const { return mChannels; }
		inline int size() const { return mWidth * mHeight * mChannels; }
		inline int mip_levels() const { return mMipLevels; }
		inline int layer_count() const { return mLayerCount; }
		inline void* pixels() const { return pPixels; }
		inline bool is_cube() const { return bIsCube; }
		inline virtual size_t offset(int level, int layer, int faceSlice) {
			return 0;
		}

	protected:
		Texture(std::filesystem::path path) {}
		void* pPixels{nullptr};
		int mWidth{0}, mHeight{0}, mChannels{0}, mMipLevels{1}, mLayerCount{1};
		bool bIsCube{false};
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};
	};

	class STB_Texture final : public Texture {
	public:
		friend TextureImporter;
		~STB_Texture();

	private:
		STB_Texture(std::filesystem::path path);
	};

	class KTX_Texture final : public Texture {
	public:
		friend TextureImporter;
		~KTX_Texture();
        size_t offset(int level, int layer, int faceSlice) override;

	private:
		KTX_Texture(std::filesystem::path path);
		ktxTexture* mTexture;
	};

	class TextureImporter {
	public:
		static Texture import(std::filesystem::path path);
	};
} // namespace asset
