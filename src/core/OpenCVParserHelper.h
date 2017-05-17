/**
 * @file   OpenCVParserHelper.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.05
 *
 * @brief  Declaration of helper methods for parsing OpenCV XML files.
 */

#pragma once

#include "main.h"
#include <external/tinyxml2.h>

namespace viscom {

    class OpenCVParserHelper final
    {
    public:
        static void LoadXMLDocument(const std::string& docName, const std::string& filename, tinyxml2::XMLDocument& doc);
        static std::string ParseTextString(tinyxml2::XMLElement* element);
        template<typename T> static T ParseText(tinyxml2::XMLElement* element);
        template<typename T> static T Parse(tinyxml2::XMLElement* element);

        static std::vector<glm::vec2> ParseVector2f(tinyxml2::XMLElement* element);
        static std::vector<glm::vec3> ParseVector3f(tinyxml2::XMLElement* element);

        static glm::vec2 Parse2f(tinyxml2::XMLElement* element);

    private:
        OpenCVParserHelper() = default;
        ~OpenCVParserHelper() = default;
    };

    template <typename T>
    T OpenCVParserHelper::ParseText(tinyxml2::XMLElement* element)
    {
        std::stringstream str(ParseTextString(element));
        T result;
        str >> result;
        return result;
    }

    template <typename T>
    T OpenCVParserHelper::Parse(tinyxml2::XMLElement* element)
    {
        std::stringstream str(element->GetText());
        T result;
        str >> result;
        return result;
    }
}
