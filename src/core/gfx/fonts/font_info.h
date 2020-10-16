/**
 * @file   font_info.h
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2020.10.13
 *
 * @brief  Declaration of the types used with font rendering.
 */

#pragma once

#include <string>
#include <vector>
#include <array>

namespace viscom::font {

    struct font_char
    {
        std::size_t id = 0;
        std::size_t index = 0;
        char c = 0;
        std::size_t width = 0;
        std::size_t height = 0;
        std::int64_t xoffset = 0;
        std::int64_t yoffset = 0;
        std::size_t xadvance = 0;
        std::size_t chnl = 0;
        std::size_t x = 0;
        std::size_t y = 0;
        std::size_t page = 0;
    };

    struct font_info
    {
        std::string face;
        std::size_t size = 0;
        std::size_t bold = 0;
        std::size_t italic = 0;
        std::vector<std::string> charset;
        std::size_t unicode = 0;
        std::size_t stretchH = 0;
        std::size_t smooth = 0;
        std::size_t aa = 0;
        std::array<std::size_t, 4> padding;
        std::array<std::size_t, 2> spacing;
    };

    struct font_common
    {
        std::size_t lineHeight = 0;
        std::size_t base = 0;
        std::size_t scaleW = 0;
        std::size_t scaleH = 0;
        std::size_t pages = 0;
        std::size_t packed = 0;
        std::size_t alphaChnl = 0;
        std::size_t redChnl = 0;
        std::size_t greenChnl = 0;
        std::size_t blueChnl = 0;
    };

    struct font_distance_field
    {
        std::string fieldType;
        std::size_t distanceRange = 0;
    };

    struct font_kerning
    {
        std::size_t first;
        std::size_t second;
        std::size_t amount;
    };

    struct font
    {
        std::vector<std::string> pages;
        std::vector<font_char> chars;
        font_info info;
        font_common common;
        font_distance_field distanceField;
        std::vector<font_kerning> kernings;
    };
}
