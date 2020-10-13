/**
 * @file   Font.cpp
 * @author Sebastian Maisch <sebastian.maisch@googlemail.com>
 * @date   2020.10.13
 *
 * @brief  Implementation of the font resource
 */

#include "Font.h"
#include <core/FrameworkInternal.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

namespace viscom::font {

    void to_json(nlohmann::json& j, const font_char& f) {
        j = nlohmann::json{ {"id", f.id}, {"index", f.index}, {"char", f.c}, {"width", f.width},
            {"height", f.height}, {"xoffset", f.xoffset}, {"yoffset", f.yoffset},
            {"xadvance", f.xadvance}, {"chnl", f.chnl}, {"x", f.x}, {"y", f.y}, {"page", f.page} };
    }

    void from_json(const nlohmann::json& j, font_char& f) {
        j.at("id").get_to(f.id);
        j.at("index").get_to(f.index);
        j.at("char").get_to(f.c);
        j.at("width").get_to(f.width);
        j.at("height").get_to(f.height);
        j.at("xoffset").get_to(f.xoffset);
        j.at("yoffset").get_to(f.yoffset);
        j.at("xadvance").get_to(f.xadvance);
        j.at("chnl").get_to(f.chnl);
        j.at("x").get_to(f.x);
        j.at("y").get_to(f.y);
        j.at("page").get_to(f.page);
    }

    void to_json(nlohmann::json& j, const font_info& f) {
        j = nlohmann::json{ {"face", f.face}, {"size", f.size}, {"bold", f.bold}, {"italic", f.italic},
            {"charset", f.charset}, {"unicode", f.unicode}, {"stretchH", f.stretchH}, {"smooth", f.smooth},
            {"aa", f.aa}, {"padding", f.padding}, {"spacing", f.spacing} };
    }

    void from_json(const nlohmann::json& j, font_info& f) {
        j.at("face").get_to(f.face);
        j.at("size").get_to(f.size);
        j.at("bold").get_to(f.bold);
        j.at("italic").get_to(f.italic);
        j.at("charset").get_to(f.charset);
        j.at("unicode").get_to(f.unicode);
        j.at("stretchH").get_to(f.stretchH);
        j.at("smooth").get_to(f.smooth);
        j.at("aa").get_to(f.aa);
        j.at("padding").get_to(f.padding);
        j.at("spacing").get_to(f.spacing);
    }

    void to_json(nlohmann::json& j, const font_common& f) {
        j = nlohmann::json{ {"lineHeight", f.lineHeight}, {"base", f.base}, {"scaleW", f.scaleW},
            {"scaleH", f.scaleH}, {"pages", f.pages}, {"packed", f.packed}, {"alphaChnl", f.alphaChnl},
            {"redChnl", f.redChnl}, {"greenChnl", f.greenChnl}, {"blueChnl", f.blueChnl} };
    }

    void from_json(const nlohmann::json& j, font_common& f) {
        j.at("lineHeight").get_to(f.lineHeight);
        j.at("base").get_to(f.base);
        j.at("scaleW").get_to(f.scaleW);
        j.at("scaleH").get_to(f.scaleH);
        j.at("pages").get_to(f.pages);
        j.at("packed").get_to(f.packed);
        j.at("alphaChnl").get_to(f.alphaChnl);
        j.at("redChnl").get_to(f.redChnl);
        j.at("greenChnl").get_to(f.greenChnl);
        j.at("blueChnl").get_to(f.blueChnl);
    }

    void to_json(nlohmann::json& j, const font_distance_field& f) {
        j = nlohmann::json{ {"fieldType", f.fieldType}, {"distanceRange", f.distanceRange} };
    }

    void from_json(const nlohmann::json& j, font_distance_field& f) {
        j.at("fieldType").get_to(f.fieldType);
        j.at("distanceRange").get_to(f.distanceRange);
    }

    void to_json(nlohmann::json& j, const font_kerning& f) {
        j = nlohmann::json{ {"first", f.first}, {"second", f.second}, {"amount", f.amount} };
    }

    void from_json(const nlohmann::json& j, font_kerning& f) {
        j.at("first").get_to(f.first);
        j.at("second").get_to(f.second);
        j.at("amount").get_to(f.amount);
    }

    void to_json(nlohmann::json& j, const font& f) {
        j = nlohmann::json{ {"pages", f.pages}, {"chars", f.chars}, {"info", f.info}, {"common", f.common}, {"distanceField", f.distanceField}, {"kernings", f.kernings} };
    }

    void from_json(const nlohmann::json& j, font& f) {
        j.at("pages").get_to(f.pages);
        j.at("chars").get_to(f.chars);
        j.at("info").get_to(f.info);
        j.at("common").get_to(f.common);
        j.at("distanceField").get_to(f.distanceField);
        j.at("kernings").get_to(f.kernings);
    }

}

namespace viscom {

    Font::Font(const std::string& fontName, FrameworkInternal* node, bool synchronize /*= false*/):
        Resource(fontName, ResourceType::Font, node, synchronize),
        fontName_{ fontName }
    {
    }

    void Font::Load(std::optional<std::vector<std::uint8_t>>& data)
    {
        auto fontBaseFilename = "fonts/" + fontName_ + "/" + fontName_;
        auto fontJsonFilename = fontBaseFilename + ".json";
        auto filename = FindResourceLocation(fontJsonFilename);

        std::filesystem::path basePath = std::filesystem::path{ filename }.parent_path();

        std::ifstream inStream(filename);
        nlohmann::json j;
        inStream >> j;

        LoadFromJSON(j.dump());

        for (const auto& page : fontInfo_.pages) {
            if (IsSynchronized()) {
                pages_.emplace_back(GetAppNode()->GetTextureManager().GetSynchronizedResource((basePath / page).string()));
            }
            else {
                pages_.emplace_back(GetAppNode()->GetTextureManager().GetResource((basePath / page).string()));
            }
        }

        if (data.has_value()) {
            data->clear();
            nlohmann::json j_data;
            j_data["basePath"] = basePath.string();
            j_data["font"] = j;

            std::string json_dump = j_data.dump();
            data->resize(json_dump.size());
            auto dataptr = data->data();
            memcpy(dataptr, json_dump.data(), json_dump.size());
        }
    }

    void Font::LoadFromMemory(const void* data, std::size_t size)
    {
        std::string json_dump;
        json_dump.resize(size);
        memcpy(json_dump.data(), data, size);

        nlohmann::json j_data = nlohmann::json::parse(json_dump);
        std::filesystem::path basePath{ j_data["basePath"].get<std::string>() };
        LoadFromJSON(j_data["font"].dump());

        for (const auto& page : fontInfo_.pages) {
            pages_.emplace_back(GetAppNode()->GetTextureManager().GetSynchronizedResource((basePath / page).string()));
        }
    }

    void Font::LoadFromJSON(const std::string& jsonString)
    {
        nlohmann::json j = nlohmann::json::parse(jsonString);
        fontInfo_ = j.get<font::font>();

        // sort chars
        std::size_t max_id = 0;
        for (const auto& charInfo : fontInfo_.chars) {
            max_id = std::max(max_id, charInfo.id);
        }
        fontInfoGPU_.resize(max_id);
        kerningGPU_.resize(max_id * max_id, 0);

        for (const auto& charInfo : fontInfo_.chars) {
            fontInfoGPU_[charInfo.id].width = static_cast<unsigned int>(charInfo.width);
            fontInfoGPU_[charInfo.id].height = static_cast<unsigned int>(charInfo.height);
            fontInfoGPU_[charInfo.id].xoffset = static_cast<unsigned int>(charInfo.xoffset);
            fontInfoGPU_[charInfo.id].yoffset = static_cast<unsigned int>(charInfo.yoffset);
            fontInfoGPU_[charInfo.id].xadvance = static_cast<unsigned int>(charInfo.xadvance);
            fontInfoGPU_[charInfo.id].chnl = static_cast<unsigned int>(charInfo.chnl);
            fontInfoGPU_[charInfo.id].x = static_cast<unsigned int>(charInfo.x);
            fontInfoGPU_[charInfo.id].y = static_cast<unsigned int>(charInfo.y);
            fontInfoGPU_[charInfo.id].page = static_cast<unsigned int>(charInfo.page);
        }

        for (const auto& kerning : fontInfo_.kernings) {
            kerningGPU_[kerning.second * max_id + kerning.first] = static_cast<unsigned int>(kerning.amount);
        }
    }

    Font::~Font() noexcept
    {
    }

}
