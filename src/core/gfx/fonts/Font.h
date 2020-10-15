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
            glm::uvec2 texture_position = glm::uvec2{ 0 };
            glm::uvec2 texture_size = glm::uvec2{ 0 };
            glm::uvec2 offset = glm::uvec2{ 0 };
            unsigned int chnl = 0;
            unsigned int page = 0;
        };

        struct font_info_gpu {
            unsigned int line_height = 0;
            unsigned int base = 0;
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

        void Initialize();

        void Load(std::optional<std::vector<std::uint8_t>>& data) override;
        void LoadFromMemory(const void* data, std::size_t size) override;

        const font::font& GetFontInfo() const { return fontInfo_; }
        const std::vector<unsigned int>& GetXAdvance() const { return x_advance_; }
        unsigned int GetKerning(std::size_t first, std::size_t second) const { return kerning_[second * maxCharId_ + first]; }
        const std::vector<std::shared_ptr<Texture>>& GetPages() const { return pages_; }
        const std::vector<font::font_char_gpu>& GetFontCharsGPU() const { return fontCharsGPU_; }
        const font::font_info_gpu& GetFontInfoGPU() const { return fontInfoGPU_; }
        const std::vector<unsigned int>& GetKerningGPU() const { return kerningGPU_; }
        GLuint GetFontMetricsUBO() const { return fontMetricsUBO_; }
        GLuint GetCharMetricsSBO() const { return charMetricsSBO_; }
        GLuint GetKerningTexture() const { return kerningTexture_; }

    private:
        void LoadFromJSON(const std::string& jsonString);
        void InitGPU();

        /** The font file name. */
        std::string fontName_;
        /** The maximum character id in the font. */
        std::size_t maxCharId_ = 0;
        /** Basic information about the font. */
        font::font fontInfo_;
        /** Width of each char. */
        std::vector<unsigned int> x_advance_;
        /** Kerning combinations. */
        std::vector<unsigned int> kerning_;
        /** Basic information about the font in GPU format. */
        font::font_info_gpu fontInfoGPU_;
        /** Basic information about the font in GPU format. */
        std::vector<font::font_char_gpu> fontCharsGPU_;
        /** Kerning information for the GPU. */
        std::vector<unsigned int> kerningGPU_;

        /** Font texture pages. */
        std::vector<std::shared_ptr<Texture>> pages_;
        /** Font metrics UBO. */
        GLuint fontMetricsUBO_ = 0;
        /** Char metrics SBO. */
        GLuint charMetricsSBO_ = 0;
        /** Kerning texture. */
        GLuint kerningTexture_ = 0;
    };
}
