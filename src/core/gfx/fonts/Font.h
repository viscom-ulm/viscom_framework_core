/**
 * @file   Font.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2020.10.13
 *
 * @brief  Declaration of a font resource.
 */

#pragma once

#include "core/resources/Resource.h"
#include "font_info.h"

namespace viscom {

    class Texture;

    namespace font {
        struct font_char_gpu {
            unsigned int width = 0;
            unsigned int height = 0;
            unsigned int xoffset = 0;
            unsigned int yoffset = 0;

            unsigned int xadvance = 0;
            unsigned int chnl = 0;
            unsigned int x = 0;
            unsigned int y = 0;

            unsigned int page = 0;
        };
    }

    class Font final : public Resource
    {
    public:
        Font(const std::string& fontName, FrameworkInternal* node, bool synchronize = false);
        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        Font(Font&&) noexcept = delete;
        Font& operator=(Font&&) noexcept = delete;
        ~Font() noexcept override;

        void Load(std::optional<std::vector<std::uint8_t>>& data) override;
        void LoadFromMemory(const void* data, std::size_t size) override;

        const font::font& GetFontInfo() const { return fontInfo_; }
        const std::vector<std::shared_ptr<Texture>>& GetPages() const { return pages_; }
        const std::vector<font::font_char_gpu>& GetFontCharsGPU() const { return fontInfoGPU_; }
        const std::vector<unsigned int>& GetKerningGPU() const { return kerningGPU_; }

    private:
        void LoadFromJSON(const std::string& jsonString);

        /** The font file name. */
        std::string fontName_;
        /** Basic information about the font. */
        font::font fontInfo_;
        /** Basic information about the font in GPU format. */
        std::vector<font::font_char_gpu> fontInfoGPU_;
        /** Kerning information for the GPU. */
        std::vector<unsigned int> kerningGPU_;

        /** Font texture pages. */
        std::vector<std::shared_ptr<Texture>> pages_;
    };
}
