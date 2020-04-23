/**
 * @file   OpenCVParserHelper.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.05
 *
 * @brief  Implementation of helper methods for loading OpenCV XML files.
 */

#include "OpenCVParserHelper.h"

namespace viscom {

    void OpenCVParserHelper::LoadXMLDocument(const std::string& docName, const std::string& filename, tinyxml2::XMLDocument& doc)
    {
        auto result = doc.LoadFile(filename.c_str());

        if (result != tinyxml2::XML_SUCCESS) {
            spdlog::critical("{} file not found ({}).", docName, filename);
            throw std::runtime_error(docName + " file not found.");
        }
    }

    std::string OpenCVParserHelper::ParseTextString(tinyxml2::XMLElement* element)
    {
        std::string result = element->GetText();
        return result.substr(1, result.size() - 2);
    }

    std::vector<float> OpenCVParserHelper::ParseVectorf(tinyxml2::XMLElement * element)
    {
        std::vector<float> result;

        std::string typeId = element->Attribute("type_id");
        if (typeId != "opencv-matrix") throw std::runtime_error("Not a OpenCV matrix.");

        if (1 != Parse<int>(element->FirstChildElement("cols"))) throw std::runtime_error("Too many columns.");
        std::string dtStr = element->FirstChildElement("dt")->GetText();
        if ("f" != dtStr) throw std::runtime_error("Wrong type.");
        auto rowsVal = element->FirstChildElement("rows"); _unused(rowsVal);
        result.resize(Parse<size_t>(element->FirstChildElement("rows")));

        std::stringstream dataStream(element->FirstChildElement("data")->GetText());
        float x;
        size_t i = 0;
        while (dataStream >> x) {
            result[i++] = x;
        }
        return result;
    }

    std::vector<glm::vec2> OpenCVParserHelper::ParseVector2f(tinyxml2::XMLElement* element)
    {
        std::vector<glm::vec2> result;

        std::string typeId = element->Attribute("type_id");
        if (typeId != "opencv-matrix") throw std::runtime_error("Not a OpenCV matrix.");

        if (1 != Parse<int>(element->FirstChildElement("cols"))) throw std::runtime_error("Too many columns.");
        if ("2f" != ParseTextString(element->FirstChildElement("dt"))) throw std::runtime_error("Wrong type.");

        result.resize(Parse<size_t>(element->FirstChildElement("rows")));

        std::stringstream dataStream(element->FirstChildElement("data")->GetText());
        float x, y;
        size_t i = 0;
        while (dataStream >> x >> y) {
            result[i++] = glm::vec2(x, y);
        }
        return result;
    }

    std::vector<glm::vec3> OpenCVParserHelper::ParseVector3f(tinyxml2::XMLElement* element)
    {
        std::vector<glm::vec3> result;

        std::string typeId = element->Attribute("type_id");
        if (typeId != "opencv-matrix") throw std::runtime_error("Not a OpenCV matrix.");

        if (1 != Parse<int>(element->FirstChildElement("cols"))) throw std::runtime_error("Too many columns.");
        if ("3f" != ParseTextString(element->FirstChildElement("dt"))) throw std::runtime_error("Wrong type.");

        result.resize(Parse<size_t>(element->FirstChildElement("rows")));

        std::stringstream dataStream(element->FirstChildElement("data")->GetText());
        float x, y, z;
        size_t i = 0;
        while (dataStream >> x >> y >> z) {
            result[i++] = glm::vec3(x, y, z);
        }
        return result;
    }

    glm::vec2 OpenCVParserHelper::Parse2f(tinyxml2::XMLElement* element)
    {
        std::stringstream dataStream(element->GetText());
        float x, y;
        dataStream >> x >> y;
        return glm::vec2(x, y);
    }
}
