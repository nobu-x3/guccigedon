#pragma once

#include <concepts>
#include <filesystem>
#include "core/types.h"

namespace asset {
    class TextureImporter;

    // Should never be constructed manually,
    // use TextureImporter::import, otherwise will crash or leak.
	class Texture {
	public:
        friend TextureImporter;
		virtual ~Texture(){}
        Texture(const Texture&);
        virtual Texture& operator=(const Texture&);
        Texture(Texture&&) noexcept;
        virtual Texture& operator=(Texture&&) noexcept;
		inline int width() const { return mWidth; }
		inline int height() const { return mHeight; }
		inline int channels() const { return mChannels; }
		inline void* pixels() const { return pPixels; }

    protected:
        Texture(std::filesystem::path path){}
		void* pPixels{nullptr};
		int mWidth{0}, mHeight{0}, mChannels{0};
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};
	};

	class STB_Texture final : public Texture {
	public:
        friend TextureImporter;
		~STB_Texture();
    private:
		STB_Texture(std::filesystem::path path);
	};

	class TextureImporter {
	public:
		static Texture import(std::filesystem::path path);
	};
} // namespace asset
